#include <stdio.h>
#include <stdlib.h>
#include <clearblade.h>

#define SYSTEM_KEY "928cddb30bacace5aa9df4c8cd6b"
#define SYSTEM_SECRET "928CDDB30BE0E3818FC0A2DCFD44"
#define PLATFORM_URL "http://localhost:9000"
#define MESSAGING_URL "http://localhost:9000"
#define USER_EMAIL "awmathie@iu.edu"
#define USER_PASSWORD "clearblade"

int main() {
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

	return 0;
}