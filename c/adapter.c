#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <clearblade.h>
#include <time.h>

/*
#define SYSTEM_KEY "928cddb30bacace5aa9df4c8cd6b"
#define SYSTEM_SECRET "928CDDB30BE0E3818FC0A2DCFD44"
#define PLATFORM_URL "http://localhost:9000"
#define MESSAGING_URL "http://localhost:9000"
#define USER_EMAIL "awmathie@iu.edu"
#define USER_PASSWORD "clearblade"
*/
#define STR_SIZE 50

int main(int argc, char *argv[]) {

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
			if (j < 4 && !parameterVariables[j][0]) {
				printf("%s is a required command line arguement!\n", parameters[j]);
				return 1;
			}
		}
	}
	
	char *platform_url, *messaging_url;

	if (parameterVariables[4][0]) {
		if (parameterVariables[5][0]) {
			platform_url = malloc(strlen(parameterVariables[4]) + strlen(parameterVariables[5]) + 1);
			strcpy(platform_url, parameterVariables[4]);
			strcat(platform_url, parameterVariables[5]);
		} else {
			platform_url = malloc(strlen(parameterVariables[4]) + 6);
			strcpy(platform_url, parameterVariables[4]);
			strcat(platform_url, ":9000");
		}
	} else {
		platform_url = "http://localhost:9000";
	}

	if (parameterVariables[6][0]) {
		if (parameterVariables[7][0]) {
			messaging_url = malloc(strlen(parameterVariables[6]) + strlen(parameterVariables[7]) + 1);
			strcpy(messaging_url, parameterVariables[6]);
			strcat(messaging_url, parameterVariables[7]);
		} else {
			messaging_url = malloc(strlen(parameterVariables[6]) + 6);
			strcpy(messaging_url, parameterVariables[6]);
			strcat(messaging_url, ":9000");
		}
	} else {
		messaging_url = "http://localhost:9000";
	}


	/* testing
	for (i = 0; i < numberOfParameters; i++) {
		if (parameterVariables[i][0]) {
			printf(fp, "%s is: %s\n", parameters[i], parameterVariables[i]);
		}
	}*/

	// LOG FILE
	FILE *fp = fopen("adapter.log", "a");
	time_t raw_time;
	struct tm *t;
	time(&raw_time);
	t = localtime(&raw_time);
	fprintf(fp, "adapter began at %s\n", asctime(t));


	// CONNECT TO CB
	void cbInitCallback(bool error, char *result) {
	  if(error) {
	    fprintf(fp, "ClearBlade init failed %s\n", result);
	    exit(-1);
	  } else {
	    fprintf(fp, "ClearBlade Init Succeeded\nAuth token: %s\n", result);
	  }
	}

	// paramters: (char *systemkey, char *systemsecret, char *platformurl, char *messagingurl,
	// char *devicename, char *activekey, void (*initCallback)(bool error, char *result)
	initializeClearBladeAsDevice(parameterVariables[0], parameterVariables[1], platforum_url,
		messaging_url, parameterVariables[2], parameterVariables[3], &cbInitCallback);

	free(platforum_url);
	free(messaging_url);

	fclose(fp);
	return 0;
}