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
	//printf("Connection String:\n%s\n", connectionString);

	return 0;
}

int set_proxy_main(int argc, char **argv)
{
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

	return set_proxy(argv[1]);
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
