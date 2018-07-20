#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <clearblade.h>
#include "MQTTAsync.h"

#define STR_SIZE 50

// FUNCTION PROTOTYPES
char *setAddress(char addr[], char port[], int isMessaging);
void connectToPlatform(char system_key[], char system_secret[], char device_id[], 
			   char device_key[], char platform_url[], char messaging_url[]);
void connectMQTT(char device_id[]);
void intHandler(int dummy);

// GLOBAL VARIABLES
FILE *fp;
int qos = 0;
static volatile int killAdapater = 0;

char systemKey[STR_SIZE];
char systemSecret[STR_SIZE];
char deviceId[STR_SIZE];
char deviceActiveKey[STR_SIZE];
char httpURL[STR_SIZE];
char httpPort[STR_SIZE];
char messagingURL[STR_SIZE];
char messagingPort[STR_SIZE];
char adapterSettingsCollection[STR_SIZE];
char adapterSettingsItem[STR_SIZE];
char topicRoot[STR_SIZE];
char deviceProvisionSvc[STR_SIZE];
char deviceHealthSvc[STR_SIZE];
char deviceLogsSvc[STR_SIZE];
char deviceStatusSvc[STR_SIZE];
char deviceDecommissionSvc[STR_SIZE];
char logLevel[STR_SIZE];
char logMQTT[STR_SIZE];

// MAIN
int main(int argc, char *argv[]) {

	signal(SIGINT, intHandler);

	if (argc < 5) {
		printf("[ERROR] systemKey, systemSecret, deviceID, and deviceActiveKey are required command line arguement!\n");
		return;
	}

	// COMMAND LINE ARGUEMENTS
	const char *parameters[] = { "-systemKey=", "-systemSecret=", "-deviceID=", "-deviceActiveKey=",
		"-httpURL=", "-httpPort=", "-messagingURL=", "-messagingPort=", "-adapterSettingsCollection=",
		"-adapterSettingsItem=", "-topicRoot=", "-deviceProvisionSvc=", "-deviceHealthSvc=",
		"-deviceLogsSvc=", "-deviceStatusSvc=", "-deviceDecommissionSvc=", "-logLevel=", "-logMQTT="};
	int i, j;

	memcpy(systemKey, argv[1] + strlen(parameters[0]), STR_SIZE);
	memcpy(systemSecret, argv[2] + strlen(parameters[1]), STR_SIZE);
	memcpy(deviceId, argv[3] + strlen(parameters[2]), STR_SIZE);
	memcpy(deviceActiveKey, argv[4] + strlen(parameters[3]), STR_SIZE);

	if (argc > 5) {
		memcpy(httpURL, argv[5] + strlen(parameters[4]), STR_SIZE);
	} if (argc > 6) {
		memcpy(httpPort, argv[6] + strlen(parameters[5]), STR_SIZE);
	} if (argc > 7) {
		memcpy(messagingURL, argv[7] + strlen(parameters[6]), STR_SIZE);
	} if (argc > 8) {
		memcpy(messagingPort, argv[8] + strlen(parameters[7]), STR_SIZE);
	} if (argc > 9) {
		memcpy(adapterSettingsCollection, argv[9] + strlen(parameters[8]), STR_SIZE);
	} if (argc > 10) {
		memcpy(adapterSettingsItem, argv[10] + strlen(parameters[9]), STR_SIZE);
	} if (argc > 11) {
		memcpy(topicRoot, argv[11] + strlen(parameters[10]), STR_SIZE);
	} if (argc > 12) {
		memcpy(deviceProvisionSvc, argv[12] + strlen(parameters[11]), STR_SIZE);
	} if (argc > 13) {
		memcpy(deviceHealthSvc, argv[13] + strlen(parameters[12]), STR_SIZE);
	} if (argc > 14) {
		memcpy(deviceLogsSvc, argv[14] + strlen(parameters[13]), STR_SIZE);
	} if (argc > 15) {
		memcpy(deviceStatusSvc, argv[15] + strlen(parameters[14]), STR_SIZE);
	} if (argc > 16) {
		memcpy(deviceDecommissionSvc, argv[16] + strlen(parameters[15]), STR_SIZE);
	} if (argc > 17) {
		memcpy(logLevel, argv[17] + strlen(parameters[16]), STR_SIZE);
	} if (argc > 18) {
		memcpy(logMQTT, argv[18] + strlen(parameters[17]), STR_SIZE);
	}
	
	/*
	// VALIDATING COMMAND LINE ARGS
	int numberOfParameters = sizeof(parameters) / sizeof(parameters[0]);
	char parameterVariables[numberOfParameters][STR_SIZE];

	for (i = 1; i < argc; i++) {
		for (j = 0; j < numberOfParameters; j++) {
			if (strstr(argv[i], parameters[j])) {
				memcpy(parameterVariables[j], argv[i] + strlen(parameters[j]), STR_SIZE);
			}
		}
		if (i < 5 && !parameterVariables[i-1][0]) {
			printf("[ERROR] %s is a required command line arguement!\n", parameters[i-1]);
			return 1;
		}	
	}
	*/

	char *platformUrl, *messagingUrl;

	platformUrl = setAddress(parameterVariables[4], parameterVariables[5], 0);
	messagingUrl = setAddress(parameterVariables[6], parameterVariables[7], 1);

	// CHECKING LOG LEVEL
	if (strcmp(parameterVariables[16], "error") && strcmp(parameterVariables[16], "warn") 
		&& strcmp(parameterVariables[16], "debug")) {
		printf("[ERROR] Invalid log level specified\n");
		return 1;
	}

	// INIT LOG FILE
	fp = fopen("adapter.log", "a");
	time_t raw_time;
	struct tm *t;
	time(&raw_time);
	t = localtime(&raw_time);
	fprintf(fp, "adapter began at %s", asctime(t));

	// CONNECT TO CB
	connectToPlatform(parameterVariables[0], parameterVariables[1], parameterVariables[2], parameterVariables[3], platformUrl, messagingUrl);
	// CONNECT TO MQTT BROKER
	connectMQTT(parameterVariables[2]);

	//TODO - Add your implemenation specific code here
	//At this point, the CB device client will have been
	//initialized and authenticated to the ClearBlade Platform or ClearBlade Edge
	while (!killAdapater) {
		fprintf(fp, "running...\n");
		sleep(5);
	}

	// FREEING ALLOCATED MEMORY
	free(platformUrl);
	free(messagingUrl);

	fprintf(fp, "\n");
	fclose(fp);
	return 0;
}

char *setAddress(char addr[], char port[], int isMessaging) {
	char *url;

	if (addr[0]) {
		if (port[0]) {
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

	fprintf(fp, "[DEBUG] Setting System Key to %s\n", system_key);
	fprintf(fp, "[DEBUG] Setting System Secret to %s\n", system_secret);
	fprintf(fp, "[DEBUG] Setting device ID to %s\n", device_id);
	fprintf(fp, "[DEBUG] Setting active key to %s\n", device_key);
	fprintf(fp, "[DEBUG] Setting platform URL to %s\n", platform_url);
	fprintf(fp, "[DEBUG] Setting messaging URL to %s\n", messaging_url);

	// paramters: (char *systemkey, char *systemsecret, char *platformurl, char *messagingurl,
	// char *devicename, char *activekey, void (*initCallback)(bool error, char *result)
	initializeClearBladeAsDevice(system_key, system_secret, platform_url,
		messaging_url, device_id, device_key, &cbInitCallback);
}

void connectMQTT(char device_id[]) {
	void onConnect(void* context, MQTTAsync_successData* response) {
	  	fprintf(fp, "Successful connection to MQTT Broker\n");
	  	// Get the 'finished' variable from the CB SDK and set it to 1 to stop the connect loop
	  	extern int finished;
	  	finished = 1;
	}

	int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
		fprintf(fp, "Message arrived\n");
		fprintf(fp, "Topic: %s\n", topicName);
		char *messagePayload = malloc(message->payloadlen + 1);
	 	strncpy(messagePayload, message->payload, message->payloadlen);
		messagePayload[message->payloadlen] = '\0';
		fprintf(fp, "Message: %s\n", messagePayload);

		//HANDLE MQTT MESSAGES HERE

		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);
		free(messagePayload);

	  	return 1;
	}

	void onDisconnect(void *context, char *cause) {
		fprintf(fp, "\nConnection lost\n");
		fprintf(fp, "Cause: %s\n", cause);
		fprintf(fp, "Killing adapter now\n");
		printf("MQTT connection lost, killing adapter now\n");

		killAdapater = 1;
	}

	fprintf(fp, "[DEBUG] Setting clientId to %s\n", device_id);
	fprintf(fp, "[DEBUG] Setting quality of service to %s\n", qos);

	// parameters: (char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, 
	// MQTTAsync_successData* response), int (*messageArrivedCallback)(void *context, char *topicName, 
	// int topicLen, MQTTAsync_message *message), void (*onConnLostCallback)(void *context, char *cause)
	//  bool autoReconnect)
	connectToMQTTAdvanced(device_id, qos, &onConnect, &messageArrived, &onDisconnect, true);
}

void intHandler(int dummy) {
	killAdapater = 1;
}
