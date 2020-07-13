#include <stdio.h>
#include <stdlib.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/xlogging.h"
#include "iothub_client_ll.h"
#include "iothub_message.h"
#include "iothubtransporthttp.h"
#include "iothubtransportmqtt.h"
#include "iothubtransportmqtt_websockets.h"
#include "iothub_client_options.h"
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/compile.h>
#include <mruby/data.h>
#include <mruby/dump.h>
#include <mruby/proc.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>

#ifdef _MSC_VER
extern int sprintf_s(char* dst, size_t dstSizeInBytes, const char* format, ...);
#endif // _MSC_VER

#define SET_TRUSTED_CERT_IN_SAMPLES

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

char* connectionString = NULL;

bool g_use_proxy;
HTTP_PROXY_OPTIONS g_proxy_options;

struct RClass *_class_device_client;
struct RClass *_class_message;

static void mrb_device_client_free(mrb_state *mrb, void *ptr);
static void mrb_message_free(mrb_state *mrb, void *ptr);

const static struct mrb_data_type mrb_device_client_type = { "DeviceClient", mrb_device_client_free };
const static struct mrb_data_type mrb_message_type = { "Message", mrb_message_free };

enum proxy_parse_state_t {
	proxy_parse_state_schema,
	proxy_parse_state_colonslash,
	proxy_parse_state_colonslashslash,
	proxy_parse_state_username,
	proxy_parse_state_password,
	proxy_parse_state_address,
	proxy_parse_state_port,
	proxy_parse_state_error,
};

int set_proxy(char *proxy)
{
	char *schema = NULL, *address = NULL, *port = NULL, *username = NULL, *password = NULL;
	int schema_len = 0, address_len = 0, port_len = 0, username_len = 0, password_len = 0;

	enum proxy_parse_state_t state = proxy_parse_state_schema;

	char *pos = proxy;
	schema = pos;
	for (char c = *pos; c != '\0'; pos++, c = *pos) {
		switch (state) {
		case proxy_parse_state_schema:
			if (c == ':') {
				schema_len = (int)pos - (int)schema;
				username = pos + 3;
				state = proxy_parse_state_colonslash;
			}
			break;
		case proxy_parse_state_colonslash:
			if (c == '/') {
				state = proxy_parse_state_colonslashslash;
			}
			else {
				username = schema;
				username_len = schema_len;
				schema = NULL;
				schema_len = 0;
				password = pos;
				state = proxy_parse_state_password;
				goto case_proxy_parse_state_password;
			}
			break;
		case proxy_parse_state_colonslashslash:
			if (c == '/') {
				state = proxy_parse_state_username;
			}
			else {
				state = proxy_parse_state_error;
			}
			break;
		case proxy_parse_state_username:
			if (c == ':') {
				username_len = (int)pos - (int)username;
				password = pos + 1;
				state = proxy_parse_state_password;
			}
			else if (c == '@') {
				username_len = (int)pos - (int)username;
				address = pos + 1;
				state = proxy_parse_state_address;
			}
			break;
		case proxy_parse_state_password:
		case_proxy_parse_state_password:
			if (c == '@') {
				password_len = (int)pos - (int)password;
				address = pos + 1;
				state = proxy_parse_state_address;
			}
			break;
		case proxy_parse_state_address:
			if (c == ':') {
				address_len = (int)pos - (int)address;
				port = pos + 1;
				state = proxy_parse_state_port;
			}
			break;
		case proxy_parse_state_port:
			if ((c < '0') && (c > '9')) {
				state = proxy_parse_state_error;
			}
			break;
		}

		if (state == proxy_parse_state_error)
			break;
	}

	switch (state)
	{
	case proxy_parse_state_schema:
	case proxy_parse_state_colonslash:
	case proxy_parse_state_colonslashslash:
		goto error;
	case proxy_parse_state_username:
		username_len = (int)pos - (int)username;
		address = username;
		address_len = username_len;
		username = NULL;
		username_len = 0;
		break;
	case proxy_parse_state_password:
		password_len = (int)pos - (int)password;
		address = username;
		address_len = username_len;
		username = NULL;
		username_len = 0;
		port = password;
		port_len = password_len;
		password = NULL;
		password_len = 0;
		break;
	case proxy_parse_state_address:
		address_len = (int)pos - (int)address;
		break;
	case proxy_parse_state_port:
		port_len = (int)pos - (int)port;
		break;
	case proxy_parse_state_error:
	default:
		goto error;
	}

	if (schema_len > 0) {
		if (strncmp(schema, "http", schema_len) != 0)
			goto error;
	}

	if (address_len == 0)
		goto error;

	char *buf = malloc(address_len + port_len + username_len + password_len + 4);
	if (buf == NULL) {
		printf("Memory error.\n");
		return -1;
	}

	free((char *)g_proxy_options.host_address);

	pos = buf;
	memcpy(pos, address, address_len);
	pos[address_len] = '\0';
	g_proxy_options.host_address = pos;

	pos = pos + address_len + 1;
	memcpy(pos, port, port_len);
	pos[port_len] = '\0';
	g_proxy_options.port = port_len == 0 ? 8080 : atoi(pos);

	pos = pos + port_len + 1;
	memcpy(pos, username, username_len);
	pos[username_len] = '\0';
	g_proxy_options.username = (username_len == 0) && (password_len == 0) ? NULL : pos;

	pos = pos + username_len + 1;
	memcpy(pos, password, password_len);
	pos[password_len] = '\0';
	g_proxy_options.password = (username_len == 0) && (password_len == 0) ? NULL : pos;

	g_use_proxy = true;
	return 0;
error:
	printf("proxy string parse error. 'http://[<user>:<password>@]<address>:<port>'\n");
	return -1;
}

mrb_value mrb_azure_iot_get_connection_string(mrb_state *mrb, mrb_value self)
{
	mrb_value ret;

	ret = mrb_str_new_cstr(mrb, connectionString);

	return ret;
}

mrb_value mrb_azure_iot_set_connection_string(mrb_state *mrb, mrb_value self)
{
	mrb_value cs;

	mrb_get_args(mrb, "S", &cs);

	if (mrb_nil_p(cs)) {
		free(connectionString);
		connectionString = NULL;
	}
	else {
		char *conn_str = (char *)malloc(RSTRING_LEN(cs) + 1);
		if (conn_str != NULL) {
			strncpy(conn_str, RSTRING_PTR(cs), RSTRING_LEN(cs));
			free(connectionString);
			connectionString = conn_str;
		}
	}

	return mrb_nil_value();
}

mrb_value mrb_azure_iot_get_proxy(mrb_state *mrb, mrb_value self)
{
	mrb_value ret;

	if (g_proxy_options.username != NULL) {
		int len = strlen(g_proxy_options.username) +
			strlen(g_proxy_options.password) +
			strlen(g_proxy_options.host_address) + 7 + 1 + 1 + 1 + 5 + 1;

		ret = mrb_str_buf_new(mrb, len);
		char *proxy = RSTRING_PTR(ret);

		sprintf(proxy, "http://%s:%s@%s:%d",
			g_proxy_options.username,
			g_proxy_options.password,
			g_proxy_options.host_address,
			g_proxy_options.port);

		mrb_str_resize(mrb, ret, strlen(proxy));
	}
	else if (g_proxy_options.host_address != NULL) {
		int len = strlen(g_proxy_options.host_address) + 7 + 1 + 5 + 1;

		ret = mrb_str_buf_new(mrb, len);
		char *proxy = RSTRING_PTR(ret);

		sprintf(proxy, "http://%s:%d",
			g_proxy_options.host_address,
			g_proxy_options.port);

		mrb_str_resize(mrb, ret, strlen(proxy));
	}
	else {
		return mrb_nil_value();
	}

	return ret;
}

mrb_value mrb_azure_iot_set_proxy(mrb_state *mrb, mrb_value self)
{
	mrb_value proxy;

	mrb_get_args(mrb, "S", &proxy);

	int ret = set_proxy(mrb_str_to_cstr(mrb, proxy));

	return ret == 0 ? mrb_true_value() : mrb_false_value();
}

typedef struct DEVICE_CLIENT_CONTEXT {
	mrb_state *mrb;
	mrb_value self;
	IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
	mrb_value twin;
} DEVICE_CLIENT_CONTEXT;

static IOTHUBMESSAGE_DISPOSITION_RESULT mrb_receive_message_callback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
	DEVICE_CLIENT_CONTEXT *context = (DEVICE_CLIENT_CONTEXT *)userContextCallback;
	const unsigned char *buffer;
	size_t size;

	if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
	{
		printf("unable to retrieve the message data\r\n");
	}

	printf(buffer);
	printf("\r\n");

	return IOTHUBMESSAGE_ACCEPTED;
}

mrb_value mrb_device_client_initialize(mrb_state *mrb, mrb_value self)
{
	const char *cs = NULL;
	mrb_int proto;

	mrb_get_args(mrb, "zi", &cs, &proto);

	IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
	switch (proto) {
	case 0:
		protocol = HTTP_Protocol;
		break;
	case 1:
		protocol = MQTT_Protocol;
		break;
	case 2:
		protocol = MQTT_WebSocket_Protocol;
		break;
	default:
		mrb_raise(mrb, E_RUNTIME_ERROR, "DeviceClient.new protocol");
		return mrb_nil_value();
	}

	DEVICE_CLIENT_CONTEXT *context = mrb_malloc(mrb, sizeof(DEVICE_CLIENT_CONTEXT));
	if (context == NULL) {
		mrb_raise(mrb, E_RUNTIME_ERROR, "DeviceClient.new context");
		return mrb_nil_value();
	}

	IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
	iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(cs, protocol);
	if (iotHubClientHandle == NULL) {
		mrb_free(mrb, context);
		mrb_raise(mrb, E_RUNTIME_ERROR, "DeviceClient.new ConnectionString");
		return mrb_nil_value();
	}

	context->mrb = mrb;
	context->self = self;
	context->iotHubClientHandle = iotHubClientHandle;

	DATA_TYPE(self) = &mrb_device_client_type;
	DATA_PTR(self) = context;

	if (g_use_proxy)
	{
		if (IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_HTTP_PROXY, &g_proxy_options) != IOTHUB_CLIENT_OK)
		{
			mrb_raise(mrb, E_RUNTIME_ERROR, "failure to set option \"HTTP Proxy\"\r\n");
		}
	}

	if (IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_TRUSTED_CERT, certificates) != IOTHUB_CLIENT_OK)
	{
		mrb_raise(mrb, E_RUNTIME_ERROR, "failure to set option \"TrustedCerts\"\r\n");
	}
	
	if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, mrb_receive_message_callback, context) != IOTHUB_CLIENT_OK)
	{
		mrb_raise(mrb, E_RUNTIME_ERROR, "ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
	}

	return self;
}

void mrb_device_client_free(mrb_state *mrb, void *ptr)
{
	DEVICE_CLIENT_CONTEXT *context = (DEVICE_CLIENT_CONTEXT *)ptr;
	IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = context->iotHubClientHandle;

	IoTHubClient_LL_Destroy(iotHubClientHandle);
	mrb_free(mrb, context);
}

typedef struct SEND_CALLBAK_CONTEXT {
	mrb_state *mrb;
	mrb_value message, block, arg;

} SEND_CALLBAK_CONTEXT;

static void mrb_send_confirmation_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
	SEND_CALLBAK_CONTEXT *context = (SEND_CALLBAK_CONTEXT *)userContextCallback;

	if (mrb_nil_p(context->block)) {
		return;
	}
	
	mrb_yield(context->mrb, context->block, context->arg);

	IOTHUB_MESSAGE_HANDLE ioTHubMessageHandle = (IOTHUB_MESSAGE_HANDLE)DATA_PTR(context->message);

	if (ioTHubMessageHandle != NULL) {
		IoTHubMessage_Destroy(ioTHubMessageHandle);
		DATA_PTR(context->message) = NULL;
	}

	mrb_free(context->mrb, context);
}

mrb_value mrb_device_client_send_event(mrb_state *mrb, mrb_value self)
{
	DEVICE_CLIENT_CONTEXT *context = (DEVICE_CLIENT_CONTEXT *)DATA_PTR(self);
	IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = context->iotHubClientHandle;
	mrb_value message, block, arg;

	mrb_get_args(mrb, "o|&o", &message, &block, &arg);

	if (!mrb_obj_is_kind_of(mrb, message, _class_message)) {
		LogError("DeviceClient#send_event");
		return mrb_nil_value();
	}

	IOTHUB_MESSAGE_HANDLE ioTHubMessageHandle = (IOTHUB_MESSAGE_HANDLE)DATA_PTR(message);
	SEND_CALLBAK_CONTEXT *callbakContext = mrb_malloc(mrb, sizeof(SEND_CALLBAK_CONTEXT));
	callbakContext->mrb = mrb;
	callbakContext->message = message;
	callbakContext->block = block;
	callbakContext->arg = arg;

	IOTHUB_CLIENT_RESULT result;
	result = IoTHubClient_LL_SendEventAsync(iotHubClientHandle, ioTHubMessageHandle, mrb_send_confirmation_callback, callbakContext);
	if (result != IOTHUB_CLIENT_OK) {
		LogError("DeviceClient#send_event");
		return mrb_nil_value();
	}

	return mrb_true_value();
}

mrb_value mrb_device_client_do_work(mrb_state *mrb, mrb_value self)
{
	DEVICE_CLIENT_CONTEXT *context = (DEVICE_CLIENT_CONTEXT *)DATA_PTR(self);
	IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = context->iotHubClientHandle;

	IoTHubClient_LL_DoWork(iotHubClientHandle);

	return mrb_nil_value();
}

static int mrb_receive_device_method_callback(const char *method_name, const unsigned char *payload, size_t size, unsigned char **response, size_t *resp_size, void *userContextCallback)
{
	DEVICE_CLIENT_CONTEXT *context = (DEVICE_CLIENT_CONTEXT *)userContextCallback;
	mrb_state *mrb = context->mrb;
	mrb_value twin = context->twin;
	mrb_value arg = mrb_str_new(mrb, payload, size);
	mrb_value ret;

	ret = mrb_funcall(mrb, twin, method_name, 1, arg);
	if (!mrb_string_p(ret)) {
		ret = mrb_obj_as_string(mrb, ret);
	}

	*resp_size = RSTRING_LEN(ret);
	*response = (unsigned char *)malloc(*resp_size);
	(void)memcpy(*response, RSTRING_PTR(ret), *resp_size);

	return 0;
}

static void mrb_device_twin_callback(int status_code, void *userContextCallback)
{
	DEVICE_CLIENT_CONTEXT *context = (DEVICE_CLIENT_CONTEXT *)userContextCallback;
}

static void mrb_receive_device_twin_callback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char *payload, size_t size, void *userContextCallback)
{
	DEVICE_CLIENT_CONTEXT *context = (DEVICE_CLIENT_CONTEXT *)userContextCallback;
	IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = context->iotHubClientHandle;
	mrb_state *mrb = context->mrb;
	mrb_value twin = context->twin;
	mrb_value arg = mrb_str_new(mrb, payload, size);
	mrb_value ret;

	ret = mrb_funcall(mrb, twin, "recv_twin", 1, arg);
	if (!mrb_string_p(ret)) {
		ret = mrb_obj_as_string(mrb, ret);
	}

	if (IoTHubClient_LL_SendReportedState(iotHubClientHandle, RSTRING_PTR(ret), RSTRING_LEN(ret), mrb_device_twin_callback, context) != IOTHUB_CLIENT_OK)
	{
		LogError("Failure sending data");
	}
}

mrb_value mrb_device_client_set_twin(mrb_state *mrb, mrb_value self)
{
	DEVICE_CLIENT_CONTEXT *context = (DEVICE_CLIENT_CONTEXT *)DATA_PTR(self);
	IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = context->iotHubClientHandle;
	mrb_value twin;

	mrb_get_args(mrb, "o", &twin);

	context->twin = twin;

	if (IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, mrb_receive_device_method_callback, context) != IOTHUB_CLIENT_OK)
	{
		LogError("ERROR: IoTHubClient_LL_SetDeviceMethodCallback..........FAILED!\r\n");
	}

	if (IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, mrb_receive_device_twin_callback, context) != IOTHUB_CLIENT_OK)
	{
		LogError("failure in IoTHubClient_LL_SetDeviceTwinCallback");
	}

	return mrb_nil_value();
}

mrb_value mrb_message_initialize(mrb_state *mrb, mrb_value self)
{
	unsigned char *msgText;
	mrb_int msgSize;

	mrb_get_args(mrb, "s", &msgText, &msgSize);

	IOTHUB_MESSAGE_HANDLE ioTHubMessageHandle;
	ioTHubMessageHandle = IoTHubMessage_CreateFromByteArray(msgText, msgSize);
	if (ioTHubMessageHandle == NULL) {
		LogError("Message.new");
		return mrb_nil_value();
	}

	DATA_TYPE(self) = &mrb_message_type;
	DATA_PTR(self) = ioTHubMessageHandle;

	return self;
}

void mrb_message_free(mrb_state *mrb, void *ptr)
{
	IOTHUB_MESSAGE_HANDLE ioTHubMessageHandle = (IOTHUB_MESSAGE_HANDLE)ptr;

	if (ioTHubMessageHandle != NULL)
		IoTHubMessage_Destroy(ioTHubMessageHandle);
}

mrb_value mrb_message_add_property(mrb_state *mrb, mrb_value self)
{
	IOTHUB_MESSAGE_HANDLE ioTHubMessageHandle = (IOTHUB_MESSAGE_HANDLE)DATA_PTR(self);
	const char *name, *value;

	mrb_get_args(mrb, "zz", &name, &value);

	MAP_HANDLE propMap;
	propMap = IoTHubMessage_Properties(ioTHubMessageHandle);
	if (Map_AddOrUpdate(propMap, name, value) != MAP_OK)
	{
		LogError("Message#add_property");
		return mrb_nil_value();
	}

	return mrb_nil_value();
}

void mrb_mruby_azure_iot_gem_init(mrb_state *mrb)
{
	struct RClass *_module_azure_iot;

	_module_azure_iot = mrb_define_module(mrb, "AzureIoT");

	mrb_define_const(mrb, _module_azure_iot, "HTTP", mrb_fixnum_value(0));
	mrb_define_const(mrb, _module_azure_iot, "MQTT", mrb_fixnum_value(1));
	mrb_define_const(mrb, _module_azure_iot, "MQTToWS", mrb_fixnum_value(2));

	mrb_define_module_function(mrb, _module_azure_iot, "set_connection_string", mrb_azure_iot_set_connection_string, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, _module_azure_iot, "get_connection_string", mrb_azure_iot_get_connection_string, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, _module_azure_iot, "set_proxy", mrb_azure_iot_set_proxy, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, _module_azure_iot, "get_proxy", mrb_azure_iot_get_proxy, MRB_ARGS_NONE());

	_class_device_client = mrb_define_class_under(mrb, _module_azure_iot, "DeviceClient", mrb->object_class);
	MRB_SET_INSTANCE_TT(_class_device_client, MRB_TT_DATA);
	mrb_define_method(mrb, _class_device_client, "initialize", mrb_device_client_initialize, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, _class_device_client, "send_event", mrb_device_client_send_event, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, _class_device_client, "do_work", mrb_device_client_do_work, MRB_ARGS_NONE());
	mrb_define_method(mrb, _class_device_client, "set_twin", mrb_device_client_set_twin, MRB_ARGS_REQ(1));

	_class_message = mrb_define_class_under(mrb, _module_azure_iot, "Message", mrb->object_class);
	MRB_SET_INSTANCE_TT(_class_message, MRB_TT_DATA);
	mrb_define_method(mrb, _class_message, "initialize", mrb_message_initialize, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, _class_message, "add_property", mrb_message_add_property, MRB_ARGS_REQ(2));

	platform_init();
}

void mrb_mruby_azure_iot_gem_final(mrb_state *mrb)
{
	platform_deinit();
}
