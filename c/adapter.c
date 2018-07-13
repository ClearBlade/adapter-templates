#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <clearblade.h>
#include "MQTTAsync.h"

#define STR_SIZE 50

// pointer to log file
FILE *fp;
// quality of service
int qos = 0;
// if we disconnect from MQTT or press control-c, kill the adapter
static volatile int killAdapater = 0;

void intHandler(int dummy) {
	killAdapater = 1;
}

int main(int argc, char *argv[]) {

	signal(SIGINT, intHandler);

	// COMMAND LINE ARGUEMENTS
	const char *parameters[] = { "-systemKey=", "-systemSecret=", "-deviceID=", "-deviceActiveKey=",
		"-httpURL=", "-httpPort=", "-messagingURL=", "-messagingPort=", "-adapterSettingsCollection=",
		"-adapterSettingsItem=", "-topicRoot=", "-deviceProvisionSvc=", "-deviceHealthSvc=",
		"-deviceLogsSvc=", "-deviceStatusSvc=", "-deviceDecommissionSvc=", "-logLevel=", "-logMQTT="};
	int i, j, numberOfParameters = sizeof(parameters) / sizeof(parameters[0]);

	// REQURED: systemKey 0, systemSecret 1, deviceId 2, deviceActiveKey 3
	char parameterVariables[numberOfParameters][STR_SIZE];
	memset(parameterVariables, 0, sizeof(parameterVariables));

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
	
	char *platformUrl, *messagingUrl;
	int platFree = 1, messFree = 1;

	if (parameterVariables[4][0]) {
		if (parameterVariables[5][0]) {
			platformUrl = malloc(strlen(parameterVariables[4]) + strlen(parameterVariables[5]) + 1);
			strcpy(platformUrl, parameterVariables[4]);
			strcat(platformUrl, parameterVariables[5]);
		} else {
			platformUrl = malloc(strlen(parameterVariables[4]) + 6);
			strcpy(platformUrl, parameterVariables[4]);
			strcat(platformUrl, ":9000");
		}
	} else {
		platFree = 0;
		platformUrl = "http://localhost:9000";
	}

	if (parameterVariables[6][0]) {
		if (parameterVariables[7][0]) {
			messagingUrl = malloc(strlen(parameterVariables[6]) + strlen(parameterVariables[7]) + 1);
			strcpy(messagingUrl, parameterVariables[6]);
			strcat(messagingUrl, parameterVariables[7]);
		} else {
			messagingUrl = malloc(strlen(parameterVariables[6]) + 6);
			strcpy(messagingUrl, parameterVariables[6]);
			strcat(messagingUrl, ":9000");
		}
	} else {
		messFree = 0;
		messagingUrl = "http://localhost:9000";
	}

	// 16 is log level
	if (strcmp(parameterVariables[16], "error") && strcmp(parameterVariables[16], "warn") 
		&& strcmp(parameterVariables[16], "debug")) {
		printf("[ERROR] Invalid log level specified\n");
		return 1;
	}

	// LOG FILE
	fp = fopen("adapter.log", "a");
	time_t raw_time;
	struct tm *t;
	time(&raw_time);
	t = localtime(&raw_time);
	fprintf(fp, "adapter began at %s", asctime(t));

	// CONNECT TO CB
	connectToPlatform(parameterVariables[0], parameterVariables[1], parameterVariables[2], 
					      parameterVariables[3], platformUrl, messagingUrl);
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
	if (platFree) {
		free(platformUrl);
	}
	if (messFree) {
		free(messagingUrl);
	}

	fprintf(fp, "\n");
	fclose(fp);
	return 0;
}

void connectToPlatform(char *system_key, char *system_secret, char *device_id, 
						   char *device_key, char *platform_url, char *messaging_url) {

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

void connectMQTT(char *device_id) {
	char *adapterName = "adapter";
	char *clientID = malloc(strlen(adapterName) + 1 + strlen(device_id));
	strcpy(clientID, adapterName);
	strcat(clientID, device_id);

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

	  MQTTAsync_freeMessage(&message);
	  MQTTAsync_free(topicName);
	  free(messagePayload);

	  return 1;
	}

	void onDisconnect(void *context, char *cause) {
	    fprintf(fp, "\nConnection lost\n");
	  	fprintf(fp, "Cause: %s\n", cause);
	  	fprintf(fp, "Ending %s now\n", adapterName);

	  	killAdapater = 1;
	}

	// parameters: (char *clientId, int qualityOfService, void (*mqttOnConnect)(void* context, 
	// MQTTAsync_successData* response), int (*messageArrivedCallback)(void *context, char *topicName, 
	// int topicLen, MQTTAsync_message *message), void (*onConnLostCallback)(void *context, char *cause))
	connectToMQTTAdvanced(clientID, qos, &onConnect, &messageArrived, &onDisconnect, 1);
	free(clientID);
}