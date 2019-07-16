#include <stdio.h>
#include <stdlib.h>

#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothub_client_ll.h"
#include "iothub_message.h"
#include "iothubtransporthttp.h"
#include "iothubtransportmqtt.h"
#include "iothubtransportmqtt_websockets.h"
#include "iothub_client_options.h"
#include "serializer.h"
#include "pinkit.h"
#include "client.h"

extern char* connectionString;

extern bool g_use_proxy;
extern HTTP_PROXY_OPTIONS g_proxy_options;

int set_cs_main(int argc, char **argv)
{
	if (argc < 2) {
		printf("usage:\n%s <connection string>\n", argv[0]);
		return 0;
	}

	int len = strlen(argv[1]);
	if (len < 70) {
		printf("String containing Hostname, Device Id & Device Key in the format:\n");
		printf("  HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>\n");
		printf("  HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>\n");
		return 0;
	}

	char *conn_str = (char *)malloc(len + 1);
	if (conn_str == NULL) {
		printf("OOM while creating connection string\n");
		return 1;
	}

	memcpy(conn_str, argv[1], len);
	conn_str[len] = 0;
	free(connectionString);
	connectionString = conn_str;
	printf("Connection String:\n%s\n", connectionString);

	return 0;
}

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

int set_proxy_main(int argc, char **argv)
{
	char *schema = NULL, *address = NULL, *port = NULL, *username = NULL, *password = NULL;
	int schema_len = 0, address_len = 0, port_len = 0, username_len = 0, password_len = 0;

	if (argc < 2) {
		if (g_proxy_options.username != NULL) {
			printf("HTTP_PROXY=http://%s:%s@%s:%d\n",
				g_proxy_options.username,
				g_proxy_options.password,
				g_proxy_options.host_address,
				g_proxy_options.port);
		}
		else if (g_proxy_options.host_address != NULL) {
			printf("HTTP_PROXY=http://%s:%d\n",
				g_proxy_options.host_address,
				g_proxy_options.port);
		}
		else {
			printf("HTTP_PROXY=\n");
		}
		return 0;
	}

	enum proxy_parse_state_t state = proxy_parse_state_schema;

	char *pos = argv[1];
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

int clear_proxy_main(int argc, char **argv)
{
	free((char *)g_proxy_options.host_address);
	g_proxy_options.host_address = NULL;
	g_proxy_options.port = 0;
	g_proxy_options.username = NULL;
	g_proxy_options.password = NULL;

	g_use_proxy = false;
	return 0;
}

int pinkit_main(int argc, char **argv)
{
	for (;;) {
		printf("x:%0.3f, y:%0.3f, z:%0.3f, temp:%0.1f\n",
			pinkit.accel.X, pinkit.accel.Y, pinkit.accel.Z,
			pinkit.temperature);

		ThreadAPI_Sleep(1000);
	}
	return 0;
}
