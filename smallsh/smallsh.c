#include <sys/types.h>		//pid_t, etc. 
//#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CHAR 2048
#define MAX_ARGS 512

void main() {
	char commandLine[MAX_CHAR];
	char *ptr; 
	int numArgs = 0;

	while (1) {
		printf(": ");		//use ': ' as prompt for each command line 
		fflush(stdout);	//flush output buffer immediately after each output 

		fgets(commandLine, MAX_CHAR, stdin);
		
		ptr = strtok(commandLine, " ");
		while (ptr != NULL) {
			printf("%s\n", ptr);
			ptr = strtok(NULL, " ");
			++numArgs;
		} 
		printf("%d\n", numArgs);
	}

}