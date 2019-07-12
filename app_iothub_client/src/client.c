// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>

/* This sample uses the _LL APIs of iothub_client for example purposes.
That does not mean that HTTP only works with the _LL APIs.
Simply changing the using the convenience layer (functions not having _LL)
and removing calls to _DoWork will yield the same results. */

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
#include "pinkit.h"

#ifdef _MSC_VER
extern int sprintf_s(char* dst, size_t dstSizeInBytes, const char* format, ...);
#endif // _MSC_VER

#define SET_TRUSTED_CERT_IN_SAMPLES

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
char* connectionString = NULL;

extern bool g_use_proxy;
extern const char *PROXY_ADDRESS;
extern uint16_t PROXY_PORT;

static int callbackCounter;
static bool g_continueRunning;
static char msgText[1024];
static char propText[1024];
#define MESSAGE_COUNT       5
#define DOWORK_LOOP_NUM     3


typedef struct EVENT_INSTANCE_TAG
{
	IOTHUB_MESSAGE_HANDLE messageHandle;
	size_t messageTrackingId;  // For tracking the messages within the user callback.
} EVENT_INSTANCE;

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
	int* counter = (int*)userContextCallback;
	const char* buffer;
	size_t size;
	MAP_HANDLE mapProperties;
	const char* messageId;
	const char* correlationId;
	const char* contentType;
	const char* contentEncoding;

	// Message properties
	if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
	{
		messageId = "<null>";
	}

	if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
	{
		correlationId = "<null>";
	}

	if ((contentType = IoTHubMessage_GetContentTypeSystemProperty(message)) == NULL)
	{
		contentType = "<null>";
	}

	if ((contentEncoding = IoTHubMessage_GetContentEncodingSystemProperty(message)) == NULL)
	{
		contentEncoding = "<null>";
	}

	// Message content
	if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK)
	{
		printf("unable to retrieve the message data\r\n");
	}
	else
	{
		(void)printf("Received Message [%d]\r\n Message ID: %s\r\n Correlation ID: %s\r\n Content-Type: %s\r\n Content-Encoding: %s\r\n Data: <<<%.*s>>> & Size=%d\r\n",
			*counter, messageId, correlationId, contentType, contentEncoding, (int)size, buffer, (int)size);
		// If we receive the work 'quit' then we stop running
		if (size == (strlen("quit") * sizeof(char)) && memcmp(buffer, "quit", size) == 0)
		{
			g_continueRunning = false;
		}
	}

	// Retrieve properties from the message
	mapProperties = IoTHubMessage_Properties(message);
	if (mapProperties != NULL)
	{
		const char*const* keys;
		const char*const* values;
		size_t propertyCount = 0;
		if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
		{
			if (propertyCount > 0)
			{
				size_t index;

				printf(" Message Properties:\r\n");
				for (index = 0; index < propertyCount; index++)
				{
					(void)printf("\tKey: %s Value: %s\r\n", keys[index], values[index]);
				}
				(void)printf("\r\n");
			}
		}
	}

	/* Some device specific action code goes here... */
	(*counter)++;
	return IOTHUBMESSAGE_ACCEPTED;
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
	EVENT_INSTANCE* eventInstance = (EVENT_INSTANCE*)userContextCallback;

	(void)printf("Confirmation[%d] received for message tracking id = %u with result = %s\r\n", callbackCounter, eventInstance->messageTrackingId, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));

	/* Some device specific action code goes here... */
	callbackCounter++;
	IoTHubMessage_Destroy(eventInstance->messageHandle);
}

void iothub_client_run(int proto)
{
	IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

	EVENT_INSTANCE messages[MESSAGE_COUNT];
	int msg_id = 0;
	double avgWindSpeed = 10.0;
	double minTemperature = 20.0;
	double minHumidity = 60.0;
	int receiveContext = 0;

	g_continueRunning = true;

	srand((unsigned int)time(NULL));

	callbackCounter = 0;

	if (platform_init() != 0)
	{
		(void)printf("Failed to initialize the platform.\r\n");
	}
	else
	{
		IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
		switch (proto) {
		case 0:
			(void)printf("Starting the IoTHub client sample HTTP...\r\n");
			protocol = HTTP_Protocol;
			break;
		case 1:
			(void)printf("Starting the IoTHub client sample MQTT...\r\n");
			protocol = MQTT_Protocol;
			break;
		case 2:
			(void)printf("Starting the IoTHub client sample MQTT over WebSocket...\r\n");
			protocol = MQTT_WebSocket_Protocol;
			break;
		default:
			platform_deinit();
			return;
		}

		if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, protocol)) == NULL)
		{
			(void)printf("ERROR: iotHubClientHandle is NULL!\r\n");
		}
		else
		{
			HTTP_PROXY_OPTIONS proxy_options;
			if (g_use_proxy)
			{
				proxy_options.host_address = PROXY_ADDRESS;
				proxy_options.port = PROXY_PORT;
				proxy_options.username = NULL;
				proxy_options.password = NULL;

				if (IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_HTTP_PROXY, &proxy_options) != IOTHUB_CLIENT_OK)
				{
					printf("failure to set option \"HTTP Proxy\"\r\n");
				}
			}
#if 0
			long curl_verbose = 1;
			if (IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_CURL_VERBOSE, &curl_verbose) != IOTHUB_CLIENT_OK)
			{
				printf("failure to set option \"CURL Verbose\"\r\n");
			}

			unsigned int timeout = 241000;
			// Because it can poll "after 9 seconds" polls will happen effectively // at ~10 seconds.
			// Note that for scalabilty, the default value of minimumPollingTime
			// is 25 minutes. For more information, see:
			// https://azure.microsoft.com/documentation/articles/iot-hub-devguide/#messaging
			unsigned int minimumPollingTime = 9;
			if (IoTHubClient_LL_SetOption(iotHubClientHandle, "timeout", &timeout) != IOTHUB_CLIENT_OK)
			{
				printf("failure to set option \"timeout\"\r\n");
			}

			if (IoTHubClient_LL_SetOption(iotHubClientHandle, "MinimumPollingTime", &minimumPollingTime) != IOTHUB_CLIENT_OK)
			{
				printf("failure to set option \"MinimumPollingTime\"\r\n");
			}

			bool traceOn = 1;
			if (IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_LOG_TRACE, &traceOn) != IOTHUB_CLIENT_OK)
			{
				printf("failure to set option \"log trace on\"\r\n");
			}
#endif
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
			// For mbed add the certificate information
			if (IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_TRUSTED_CERT, certificates) != IOTHUB_CLIENT_OK)
			{
				printf("failure to set option \"TrustedCerts\"\r\n");
			}
#endif // SET_TRUSTED_CERT_IN_SAMPLES
			/* Setting Message call back, so we can receive Commands. */
			if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
			{
				(void)printf("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
			}
			else
			{
				(void)printf("IoTHubClient_LL_SetMessageCallback...successful.\r\n");

				/* Now that we are ready to receive commands, let's send some messages */
				int iterator = 4000;
				double windSpeed = 0;
				double temperature = 0;
				double humidity = 0;
				do
				{
					if (iterator >= 5000)
					{
						iterator = 0;
						int msg_pos = msg_id % MESSAGE_COUNT;
						windSpeed = avgWindSpeed + (rand() % 4 + 2);
						temperature = minTemperature + (rand() % 10);
						humidity = minHumidity + (rand() % 20);
						sprintf_s(msgText, sizeof(msgText), "{\"windSpeed\":%.2f,\"temperature\":%.2f,\"humidity\":%.2f}", windSpeed, temperature, humidity);
						if ((messages[msg_pos].messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText))) == NULL)
						{
							(void)printf("ERROR: iotHubMessageHandle is NULL!\r\n");
						}
						else
						{
							MAP_HANDLE propMap;

							messages[msg_pos].messageTrackingId = msg_id;

							propMap = IoTHubMessage_Properties(messages[msg_pos].messageHandle);
							(void)sprintf_s(propText, sizeof(propText), temperature > 28 ? "true" : "false");
							if (Map_AddOrUpdate(propMap, "temperatureAlert", propText) != MAP_OK)
							{
								(void)printf("ERROR: Map_AddOrUpdate Failed!\r\n");
							}

							if (proto == 0) {
								(void)IoTHubMessage_SetContentTypeSystemProperty(messages[msg_pos].messageHandle, "application/json");
								(void)IoTHubMessage_SetContentEncodingSystemProperty(messages[msg_pos].messageHandle, "utf-8");
							}

							if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messages[msg_pos].messageHandle, SendConfirmationCallback, &messages[msg_pos]) != IOTHUB_CLIENT_OK)
							{
								(void)printf("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
							}
							else
							{
								(void)printf("IoTHubClient_LL_SendEventAsync accepted message [%d] for transmission to IoT Hub.\r\n", msg_id);
							}
							msg_id++;
						}
					}
					iterator++;

					IoTHubClient_LL_DoWork(iotHubClientHandle);
					ThreadAPI_Sleep(1);

				} while (g_continueRunning);

				(void)printf("iothub_client_sample_http has gotten quit message, call DoWork %d more time to complete final sending...\r\n", DOWORK_LOOP_NUM);
				for (size_t index = 0; index < DOWORK_LOOP_NUM; index++)
				{
					IoTHubClient_LL_DoWork(iotHubClientHandle);
					ThreadAPI_Sleep(1);
				}
			}
			IoTHubClient_LL_Destroy(iotHubClientHandle);
		}
		platform_deinit();
	}
}

void iothub_client_init()
{
	static const char *conn_str = "[device connection string]";
	int len = strlen(conn_str);
	free(connectionString);
	connectionString = malloc(len + 1);
	memcpy(connectionString, conn_str, len);
	connectionString[len] = 0;
}

int iothub_client_main(int argc, char **argv)
{
	if (connectionString == NULL) {
		printf("Not set connection string, use dps_csgen or set_cs.\n");
		return 0;
	}

	if (argc < 2) {
		iothub_client_run(1);
		return 0;
	}

	if (strcmp(argv[1], "http") == 0) {
		iothub_client_run(0);
		return 0;
	}
	else if (strcmp(argv[1], "mqtt") == 0) {
		iothub_client_run(1);
		return 0;
	}
	else if (strcmp(argv[1], "mqttows") == 0) {
		iothub_client_run(2);
		return 0;
	}

	printf("%s [http|mqtt|mqttows] \n", argv[0]);
	return 0;
}

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

int pinkit_main(int argc, char **argv)
{
	for(;;){
		printf("x:%0.3f, y:%0.3f, z:%0.3f, temp:%0.1f\n",
			pinkit.accel.X, pinkit.accel.Y, pinkit.accel.Z,
			pinkit.temperature);

		ThreadAPI_Sleep(1000);
	}
	return 0;
}
