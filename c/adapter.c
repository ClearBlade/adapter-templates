#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <clearblade.h>
#include "MQTTAsync.h"

// ENUM AND STRUCT ARRAY FOR HANDLING CL ARGS
typedef enum {systemKey, systemSecret, deviceID, deviceActiveKey, httpURL, httpPort,
messagingURL, messagingPort, adapterSettingsCollection, adapterSettingsItem, topicRoot,
deviceProvisionSvc, deviceHealthSvc, deviceLogsSvc, deviceStatusSvc, deviceDecommissionSvc,
logLevel, logMQTT} ARGUMENT;

const static struct {
    ARGUMENT    arg;
    const char *str;
} conversion [] = {
    {systemKey, "systemKey"},
    {systemSecret, "systemSecret"},
    {deviceID, "deviceID"},
    {deviceActiveKey, "deviceActiveKey"},
    {httpURL, "httpURL"},
    {httpPort, "httpPort"},
    {messagingURL, "messagingURL"},
    {messagingPort, "messagingPort"},
    {adapterSettingsCollection, "adapterSettingsCollection"},
    {adapterSettingsItem, "adapterSettingsItem"},
    {topicRoot, "topicRoot"},
    {deviceProvisionSvc, "deviceProvisionSvc"},
    {deviceHealthSvc, "deviceHealthSvc"},
    {deviceLogsSvc, "deviceLogsSvc"},
    {deviceStatusSvc, "deviceStatusSvc"},
    {deviceDecommissionSvc, "deviceDecommissionSvc"},
    {logLevel, "logLevel"},
    {logMQTT, "logMQTT"}
};

// FUNCTION PROTOTYPES
int handleCLArgs(int argc, char *argv[]);
void initLogs();
ARGUMENT str2enum (char *str);
char *setAddress(char addr[], char port[], int isMessaging);
void connectToPlatform(char system_key[], char system_secret[], char device_id[], 
			   char device_key[], char platform_url[], char messaging_url[]);
void connectMQTT(char device_id[]);
void intHandler(int dummy);

// GLOBAL VARIABLES
FILE *fp;
int qos = 0;
static volatile int killAdapater = 0;

char *system_key;
char *system_secret;
char *device_id;
char *device_active_key;
char *http_url;
char *http_port;
char *messaging_url;
char *messaging_port;
char *adapter_settings_collection;
char *adapter_settings_item;
char *topic_root;
char *device_provision_svc;
char *device_health_svc;
char *device_logs_svc;
char *device_status_svc;
char *device_decommission_svc;
char *log_level;
char *log_mqtt;

int main(int argc, char *argv[]) {

	signal(SIGINT, intHandler);

	int fail = handleCLArgs(argc, argv);
	if (fail) {
		return 1;
	}

	char *platformUrl, *messagingUrl;
	platformUrl = setAddress(http_url, http_port, 0);
	messagingUrl = setAddress(messaging_url, messaging_port, 1);

	// CHECKING LOG LEVEL
	if (strcmp(log_level, "error") && strcmp(log_level, "warn") && strcmp(log_level, "debug")) {
		printf("[ERROR] Invalid log level specified\n");
		return 1;
	}

	// INIT LOG FILE
	initLogs();

	// CONNECT TO CB
	connectToPlatform(system_key, system_secret, device_id, device_active_key, platformUrl, messagingUrl);
	// CONNECT TO MQTT BROKER
	connectMQTT(device_id);

	//TODO - Add your implemenation specific code here
	//At this point, the CB device client will have been
	//initialized and authenticated to the ClearBlade Platform or ClearBlade Edge

	while (!killAdapater) {
		
	}

	// FREEING ALLOCATED MEMORY
	free(platformUrl);
	free(messagingUrl);

	fprintf(fp, "\n");
	fclose(fp);
	return 0;
}

// FUNCTIONS

int handleCLArgs(int argc, char *argv[]) {
	if (argc < 5) {
		printf("[ERROR] systemKey, systemSecret, deviceID, and deviceActiveKey are required command line arguement!\n");
		return 1;
	}

	*argv++;
	argc--;

	while (argc--) {
		char *argName = malloc(35);
		char *val = strchr(*argv, '=');
		*val++;
		int pos = val - *argv;
		strncpy(argName, *argv, pos - 1);
		*argName++;

		switch (str2enum(argName)) {
			case 0:
				system_key = val;
				break;
			case 1:
				system_secret = val;
				break;
			case 2:
				device_id = val;
				break;
			case 3:
				device_active_key = val;
				break;
			case 4:
				http_url = val;
				break;
			case 5:
				http_port = val;
				break;
			case 6:
				messaging_url = val;
				break;
			case 7:
				messaging_port = val;
				break;
			case 8:
				adapter_settings_collection = val;
				break;
			case 9:
				adapter_settings_item = val;
				break;
			case 10:
				topic_root = val;
				break;
			case 11:
				device_provision_svc = val;
				break;
			case 12:
				device_health_svc = val;
				break;
			case 13:
				device_logs_svc = val;
				break;
			case 14:
				device_status_svc = val;
				break;
			case 15:
				device_decommission_svc = val;
				break;
			case 16:
				log_level = val;
				break;
			case 17:
				log_mqtt = val;
				break;
			default:
				printf("[ERROR] unknown argument passed in: %s %s\n", argName, val);
				return 1;
		}

		memset(argName, 0, strlen(argName));
		*argv++;
	}

	return 0;
}

void initLogs() {
	fp = fopen("adapter.log", "a");
	fflush(fp);
	time_t raw_time;
	struct tm *t;
	time(&raw_time);
	t = localtime(&raw_time);
	fprintf(fp, "adapter began at %s", asctime(t));
}

ARGUMENT str2enum (char *str)
{
     int j, max = sizeof(conversion) / sizeof(conversion[0]);
     for (j = 0;  j < max;  j++) {
	if (!strcmp (str, conversion[j].str)) {
	     //printf("conversion returning %d\n", j);
             return conversion[j].arg;
	}
     }

     printf("didn't find %s when attempting to convert. went up to %d\n", str, max);
}

int evaluate(char *str) {
	if (!strcmp(str, "true"))
		return 1;
	return 0;
}

char *setAddress(char *addr, char *port, int isMessaging) {
	char *url;

	if (addr != NULL) {
		if (port != NULL) {
			url = malloc(strlen(addr) + strlen(port) + 1);
			strcpy(url, addr);
			strcat(url, port);
		} else {
			url = malloc(strlen(addr) + 6);
			strcpy(url, addr);

			if (isMessaging) {
				strcat(url, ":1883");
			} else {
				strcat(url, ":9000");
			}
		}
	} else {
		if (isMessaging) {
			url = "localhost:1883";
		} else {
			url = "http://localhost:9000";
		}
	}

	return url;
}

void connectToPlatform(char system_key[], char system_secret[], char device_id[],
						   char device_key[], char platform_url[], char messaging_url[]) {

	void cbInitCallback(bool error, char *result) {
	  if(error) {
	    fprintf(fp, "ClearBlade init failed %s\n", result);
	    exit(-1);
	  } else {
	    fprintf(fp, "ClearBlade Init Succeeded\nAuth token: %s\n", result);
	  }
	}

	if (!strcmp(log_level, "debug")) {
		fprintf(fp, "[DEBUG] Setting System Key to %s\n", system_key);
		fprintf(fp, "[DEBUG] Setting System Secret to %s\n", system_secret);
		fprintf(fp, "[DEBUG] Setting device ID to %s\n", device_id);
		fprintf(fp, "[DEBUG] Setting active key to %s\n", device_key);
		fprintf(fp, "[DEBUG] Setting platform URL to %s\n", platform_url);
		fprintf(fp, "[DEBUG] Setting messaging URL to %s\n", messaging_url);
	}

	// paramters: (char *systemkey, char *systemsecret, char *platformurl, char *messagingurl,
	// char *devicename, char *activekey, void (*initCallback)(bool error, char *result)
	initializeClearBladeAsDevice(system_key, system_secret, platform_url,
		messaging_url, device_id, device_key, &cbInitCallback);
}

void connectMQTT(char device_id[]) {
	void onConnect(void* context, MQTTAsync_successData* response) {
		if (evaluate(log_mqtt)) {
	  		fprintf(fp, "Successful connection to MQTT Broker\n");
		}
	  	// Get the 'finished' variable from the CB SDK and set it to 1 to stop the connect loop
	  	extern int finished;
	  	finished = 1;
	}

	int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
		char *messagePayload = malloc(message->payloadlen + 1);
	 	strncpy(messagePayload, message->payload, message->payloadlen);
		messagePayload[message->payloadlen] = '\0';
		if (evaluate(log_mqtt)) {
			fprintf(fp, "Message arrived\n");
			fprintf(fp, "Topic: %s\n", topicName);
			fprintf(fp, "Message: %s\n", messagePayload);
		}

		//HANDLE MQTT MESSAGES HERE

		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);
		free(messagePayload);

	  	return 1;
	}

	void onDisconnect(void *context, char *cause) {
		if (evaluate(log_mqtt)) {
			fprintf(fp, "\nConnection lost\n");
			fprintf(fp, "Cause: %s\n", cause);
			fprintf(fp, "Killing adapter now\n");
		}
		printf("MQTT connection lost, killing adapter now\n");

		killAdapater = 1;
	}
	
	if (evaluate(log_mqtt) && !strcmp(log_level, "debug")) {
		fprintf(fp, "[DEBUG] Setting clientId to %s\n", device_id);
		fprintf(fp, "[DEBUG] Setting quality of service to %d\n", qos);
	}

	// parameters: (char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, 
	// MQTTAsync_successData* response), int (*messageArrivedCallback)(void *context, char *topicName, 
	// int topicLen, MQTTAsync_message *message), void (*onConnLostCallback)(void *context, char *cause)
	//  bool autoReconnect)
	connectToMQTTAdvanced(device_id, qos, &onConnect, &messageArrived, &onDisconnect, true);
}

void intHandler(int dummy) {
	killAdapater = 1;
	fclose(fp);
}
