#include <stdio.h>

int main(int argc, char *argv[]) {
	char *word;

	while (--argc > 0 && (*++argv)[0] == '-') {
		switch (argv[argc]) {
			case "test":
				printf("we found test!\n");
				break;
			default:
				printf("default\n");
				break;
		}
	}

	return 0;
}