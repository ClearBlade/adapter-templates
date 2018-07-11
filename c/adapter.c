#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <clearblade.h>

#define SYSTEM_KEY "928cddb30bacace5aa9df4c8cd6b"
#define SYSTEM_SECRET "928CDDB30BE0E3818FC0A2DCFD44"
#define PLATFORM_URL "http://localhost:9000"
#define MESSAGING_URL "http://localhost:9000"
#define USER_EMAIL "awmathie@iu.edu"
#define USER_PASSWORD "clearblade"
#define STR_SIZE 50

int main(int argc, char *argv[]) {
	int i, j;
	const char *parameters[] = { "-systemKey=", "-systemSecret=", "-deviceID=", "-deviceActiveKey=",
	"-httpURL=", "-httpPort=", "-messagingURL=", "-messagingPort=", "-adapterSettingsCollection=",
	"-adapterSettingsItem=", "-topicRoot=", "-deviceProvisionSvc=", "-deviceHealthSvc=",
	"-deviceLogsSvc=", "-deviceStatusSvc=", "-deviceDecommissionSvc=", "-logLevel=", "-logMQTT="};
	int numberOfParameters = sizeof(parameters) / sizeof(parameters[0]);
	printf("number of parameters: %d\n", numberOfParameters);

	// systemKey 0, systemSecret 1, deviceId 2, deviceActiveKey 3
	char parameterVariables[numberOfParameters][STR_SIZE];
	memset(parameterVariables, 0, sizeof(parameterVariables));

	for (i = 1; i < argc; i++) {
		for (j = 0; j < numberOfParameters; j++) {
			if (strstr(argv[i], parameters[j])) {
				memcpy(parameterVariables[j], argv[i] + strlen(parameters[j]), STR_SIZE);
			}
		}
	}
	
	for (i = 0; i < numberOfParameters; i++) {
		if (parameterVariables[i]) {
			printf("%s is: %s\n", parameters[i], parameterVariables[i]);
		}
	}

	/*
	FILE *fp = fopen("adapter.log", "a");

	void cbInitCallback(bool error, char *result) {
	  if(error) {
	    fprintf(fp, "ClearBlade init failed %s\n", result);
	    exit(-1);
	  } else {
	    fprintf(fp, "ClearBlade Init Succeeded\nAuth token: %s\n", result);
	  }
	}

	initializeClearBlade(SYSTEM_KEY, SYSTEM_SECRET, PLATFORM_URL, MESSAGING_URL, USER_EMAIL, USER_PASSWORD, &cbInitCallback);

	
	fprintf(fp, "heyyyyy\n");
	fclose(fp);
	*/
	return 0;
}