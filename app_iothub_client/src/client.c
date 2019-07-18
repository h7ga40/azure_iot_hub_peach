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
#include "serializer.h"
#include "serializer_devicetwin.h"
#include "pinkit.h"
#include "client.h"

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

bool g_use_proxy;
HTTP_PROXY_OPTIONS g_proxy_options;

static int callbackCounter;
static bool g_continueRunning;
static bool g_twinReport;
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

// Define the Model
BEGIN_NAMESPACE(WeatherStation);

DECLARE_MODEL(ContosoAnemometer,
WITH_DATA(double, windSpeed),
WITH_DATA(double, temperature),
WITH_DATA(double, humidity),
WITH_METHOD(quit),
WITH_METHOD(TurnFanOn),
WITH_METHOD(TurnFanOff)
);

DECLARE_STRUCT(FanSpeedD,
	int, value
);

DECLARE_STRUCT(FanSpeedR,
	int, value,
	ascii_char_ptr, status
);

DECLARE_DEVICETWIN_MODEL(AnemometerState,
WITH_REPORTED_PROPERTY(FanSpeedR, fanSpeed)
);

DECLARE_DEVICETWIN_MODEL(AnemometerSettings,
WITH_DESIRED_PROPERTY(FanSpeedD, fanSpeed, onDesiredFanSpeed)
);

END_NAMESPACE(WeatherStation);

void anemometerReportedStateCallback(int status_code, void* userContextCallback)
{
	AnemometerState *anemometer = (AnemometerState *)userContextCallback;

	printf("received states %d, reported fanSpeed = %d\n", status_code, anemometer->fanSpeed.value);
}

void onDesiredFanSpeed(void* argument)
{
	// Note: The argument is NOT a pointer to fanSpeed, but instead a pointer to the MODEL
	//       that contains fanSpeed as one of its arguments.  In this case, it
	//       is AnemometerSettings*.

	AnemometerSettings *anemometer = (AnemometerSettings *)argument;
	printf("received a new desired.fanSpeed = %d\n", anemometer->fanSpeed.value);

	g_twinReport = true;
}

METHODRETURN_HANDLE quit(ContosoAnemometer* device)
{
	(void)device;
	(void)printf("quit with Method.\r\n");

	g_continueRunning = false;

	METHODRETURN_HANDLE result = MethodReturn_Create(0, "{\"Message\":\"quit with Method\"}");
	return result;
}

METHODRETURN_HANDLE TurnFanOn(ContosoAnemometer* device)
{
	(void)device;
	(void)printf("Turning fan on with Method.\r\n");

	METHODRETURN_HANDLE result = MethodReturn_Create(1, "{\"Message\":\"Turning fan on with Method\"}");
	return result;
}

METHODRETURN_HANDLE TurnFanOff(ContosoAnemometer* device)
{
	(void)device;
	(void)printf("Turning fan off with Method.\r\n");

	METHODRETURN_HANDLE result = MethodReturn_Create(0, "{\"Message\":\"Turning fan off with Method\"}");
	return result;
}

static int ReceiveDeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback)
{
	int result;

	/*this is step  3: receive the method and push that payload into serializer (from below)*/
	char* payloadZeroTerminated = (char*)malloc(size + 1);
	if (payloadZeroTerminated == 0)
	{
		printf("failed to malloc\r\n");
		*resp_size = 0;
		*response = NULL;
		result = -1;
	}
	else
	{
		(void)memcpy(payloadZeroTerminated, payload, size);
		payloadZeroTerminated[size] = '\0';

		/*execute method - userContextCallback is of type deviceModel*/
		METHODRETURN_HANDLE methodResult = EXECUTE_METHOD(userContextCallback, method_name, payloadZeroTerminated);
		free(payloadZeroTerminated);

		if (methodResult == NULL)
		{
			printf("failed to EXECUTE_METHOD\r\n");
			*resp_size = 0;
			*response = NULL;
			result = -1;
		}
		else
		{
			/* get the serializer answer and push it in the networking stack*/
			const METHODRETURN_DATA* data = MethodReturn_GetReturn(methodResult);
			if (data == NULL)
			{
				printf("failed to MethodReturn_GetReturn\r\n");
				*resp_size = 0;
				*response = NULL;
				result = -1;
			}
			else
			{
				result = data->statusCode;
				if (data->jsonValue == NULL)
				{
					char* resp = "{}";
					*resp_size = strlen(resp);
					*response = (unsigned char*)malloc(*resp_size);
					(void)memcpy(*response, resp, *resp_size);
				}
				else
				{
					*resp_size = strlen(data->jsonValue);
					*response = (unsigned char*)malloc(*resp_size);
					(void)memcpy(*response, data->jsonValue, *resp_size);
				}
			}
			MethodReturn_Destroy(methodResult);
		}
	}
	return result;
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
	else {
		if (serializer_init(NULL) != SERIALIZER_OK)
		{
			(void)printf("Failed in serializer_init.");
		}
		else if (SERIALIZER_REGISTER_NAMESPACE(WeatherStation) == NULL)
		{
			LogError("unable to SERIALIZER_REGISTER_NAMESPACE");
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
				if (g_use_proxy)
				{
					if (IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_HTTP_PROXY, &g_proxy_options) != IOTHUB_CLIENT_OK)
					{
						printf("failure to set option \"HTTP Proxy\"\r\n");
					}
				}
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
				// For mbed add the certificate information
				if (IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_TRUSTED_CERT, certificates) != IOTHUB_CLIENT_OK)
				{
					printf("failure to set option \"TrustedCerts\"\r\n");
				}
#endif // SET_TRUSTED_CERT_IN_SAMPLES
				AnemometerState *anemometerState = IoTHubDeviceTwin_LL_CreateAnemometerState(iotHubClientHandle);
				if (anemometerState == NULL)
				{
					printf("Failure in IoTHubDeviceTwin_LL_CreateAnemometerState");
				}
				else
				{
					(void)printf("IoTHubDeviceTwin_LL_CreateAnemometerState...successful.\r\n");
				}
				AnemometerSettings *anemometerSettings = IoTHubDeviceTwin_LL_CreateAnemometerSettings(iotHubClientHandle);
				if (anemometerSettings == NULL)
				{
					printf("Failure in IoTHubDeviceTwin_LL_CreateAnemometerSettings");
				}
				else
				{
					(void)printf("IoTHubDeviceTwin_LL_CreateAnemometerSettings...successful.\r\n");
				}
				ContosoAnemometer* myWeather = CREATE_MODEL_INSTANCE(WeatherStation, ContosoAnemometer);
				if (myWeather == NULL)
				{
					(void)printf("Failed on CREATE_MODEL_INSTANCE\r\n");
				}
				else if (IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, ReceiveDeviceMethodCallback, myWeather) != IOTHUB_CLIENT_OK)
				{
					(void)printf("ERROR: IoTHubClient_LL_SetDeviceMethodCallback..........FAILED!\r\n");
				}
				else
				{
					(void)printf("IoTHubClient_LL_SetDeviceMethodCallback...successful.\r\n");
				}
				/* Setting Message call back, so we can receive Commands. */
				if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
				{
					(void)printf("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
				}
				else
				{
					(void)printf("IoTHubClient_LL_SetMessageCallback...successful.\r\n");
				}

				/* Now that we are ready to receive commands, let's send some messages */
				int iterator = 4000;
				do
				{
					if (iterator >= 5000)
					{
						iterator = 0;
						int msgPos = msg_id % MESSAGE_COUNT;
						unsigned char *msgText;
						size_t msgSize;
						myWeather->windSpeed = avgWindSpeed + (rand() % 4 + 2);
						myWeather->temperature = minTemperature + (rand() % 10);
						myWeather->humidity = minHumidity + (rand() % 20);
						if (SERIALIZE(&msgText, &msgSize, myWeather->windSpeed, myWeather->temperature, myWeather->humidity) != CODEFIRST_OK)
						{
							(void)printf("Failed to serialize\r\n");
						}
						else if ((messages[msgPos].messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, msgSize)) == NULL)
						{
							(void)printf("ERROR: iotHubMessageHandle is NULL!\r\n");
							free(msgText);
						}
						else
						{
							MAP_HANDLE propMap;

							messages[msgPos].messageTrackingId = msg_id;

							propMap = IoTHubMessage_Properties(messages[msgPos].messageHandle);
							(void)sprintf_s(propText, sizeof(propText), myWeather->temperature > 28 ? "true" : "false");
							if (Map_AddOrUpdate(propMap, "temperatureAlert", propText) != MAP_OK)
							{
								(void)printf("ERROR: Map_AddOrUpdate Failed!\r\n");
							}

							if (proto == 0) {
								(void)IoTHubMessage_SetContentTypeSystemProperty(messages[msgPos].messageHandle, "application/json");
								(void)IoTHubMessage_SetContentEncodingSystemProperty(messages[msgPos].messageHandle, "utf-8");
							}

							if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messages[msgPos].messageHandle, SendConfirmationCallback, &messages[msgPos]) != IOTHUB_CLIENT_OK)
							{
								(void)printf("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
							}
							else
							{
								(void)printf("IoTHubClient_LL_SendEventAsync accepted message [%d] for transmission to IoT Hub.\r\n", msg_id);
							}

							free(msgText);
							msg_id++;
						}
					}
					else if (g_twinReport) {
						g_twinReport = false;
						anemometerState->fanSpeed.value = anemometerSettings->fanSpeed.value;
						anemometerState->fanSpeed.status = "success";
						IoTHubDeviceTwin_LL_SendReportedStateAnemometerState(anemometerState, anemometerReportedStateCallback, anemometerState);
					}
					iterator++;

					IoTHubClient_LL_DoWork(iotHubClientHandle);
					ThreadAPI_Sleep(1);

				} while (g_continueRunning);

				(void)printf("iothub_client_run has gotten quit message, call DoWork %d more time to complete final sending...\r\n", DOWORK_LOOP_NUM);
				for (size_t index = 0; index < DOWORK_LOOP_NUM; index++)
				{
					IoTHubClient_LL_DoWork(iotHubClientHandle);
					ThreadAPI_Sleep(1);
				}

				if (anemometerSettings != NULL)
					IoTHubDeviceTwin_LL_DestroyAnemometerSettings(anemometerSettings);
				if (anemometerState != NULL)
					IoTHubDeviceTwin_LL_DestroyAnemometerState(anemometerState);
				if (myWeather != NULL)
					DESTROY_MODEL_INSTANCE(myWeather);
				IoTHubClient_LL_Destroy(iotHubClientHandle);
			}
			serializer_deinit();
		}
		platform_deinit();
	}
}

void iothub_client_init()
{
#if 0
	char *set_cs_argv[2] = {
		"set_cs",
		"[device connection string]"
	};
	set_cs_main(2, set_cs_argv);

	char *set_proxy_argv[2] = {
		"set_proxy",
		"example.com:8080"
	};
	set_proxy_main(2, set_proxy_argv);
#endif
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
