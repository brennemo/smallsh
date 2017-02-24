#include <sys/types.h>		//pid_t, etc. 
//#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CHAR 2048
#define MAX_ARGS 512

void smallsh_exit();
void smallsh_cd();
void smallsh_status();

int main() {
	char commandLine[MAX_CHAR];
	char *ptr; 
	int i, numArgs;			 
	char *args[MAX_ARGS];

	while (1) {
		numArgs = 0; 

		printf(": ");		//use ': ' as prompt for each command line 
		fflush(stdout);	//flush output buffer immediately after each output 

		fgets(commandLine, MAX_CHAR, stdin);
		
		ptr = strtok(commandLine, " \n");
		printf("command: %s\n", ptr);
		
		while (ptr != NULL) {
			if (strcmp(ptr, ">") == 0) {
				printf("input!\n");
			}   
			
			else if (strcmp(ptr, "<") == 0) {
				printf("output!\n");
			}
			else if (strcmp(ptr, "&") == 0) {
				printf("background process!\n");
			}
			args[numArgs] = ptr;		//store 
			ptr = strtok(NULL, " \n");
			numArgs++;
		} 
		args[numArgs] = ptr;			//last argument 

		printf("# Arguments: %d\n", numArgs);
		for (i = 0; i < numArgs; i++) {
			printf("%s ", args[i]);
		}
		printf("\n");
	}

	return 0;

}		

void smallsh_exit() {

}

void smallsh_cd() {

}

void smallsh_status() {

}