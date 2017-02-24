#include <sys/types.h>		//pid_t, etc. 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR 2048
#define MAX_ARGS 512

int main() {
	char commandLine[MAX_CHAR];
	char *ptr; //*inputFile, *outputFile; 
	int i, numArgs; //inputIndex, outputIndex;
	char *args[MAX_ARGS];

	while (1) {
		numArgs = 0; 
		//inputIndex = outputIndex = -1;

		printf(": ");		//use ': ' as prompt for each command line 
		fflush(stdout);	//flush output buffer immediately after each output 

		fgets(commandLine, MAX_CHAR, stdin);
		
		ptr = strtok(commandLine, " \n");			
		printf("command: %s\n", ptr);
		
		while (ptr != NULL) {
			/*
			if (strcmp(ptr, ">") == 0) {
				printf("input!\n");
				inputIndex = numArgs;
			}   
			
			else if (strcmp(ptr, "<") == 0) {
				printf("output!\n");
				outputIndex = numArgs; 
			}
			*/
			/*
			else if (strcmp(ptr, "&") == 0) {
				printf("background process!\n");
			}  
			*/
			args[numArgs] = ptr;			//store command/argument in array 
			ptr = strtok(NULL, " \n");		 
			numArgs++;
		} 
		args[numArgs] = ptr;			//last argument 

		printf("# Arguments: %d\n", numArgs);
		for (i = 0; i < numArgs; i++) {
			printf("%s ", args[i]);
		}
		printf("\n");

		//Check for background process at end of command 
		if (strcmp(args[numArgs - 1], "&") == 0) {
			printf("background process!\n");
		}
			
		//Check for built-in commands

		if (strcmp(args[0], "exit") == 0) {
			if (numArgs == 1)			//check # of args  
				exit(0);
			else
				printf("");
		}
		else if (strcmp(args[0], "cd") == 0) {
			if (numArgs == 1 || numArgs == 2)
				//change to directory in HOME environment variable 
				if (numArgs == 1) {
					printf("%s\n", args[0]);
					if (chdir(getenv("HOME"))) {
						perror("Cannot find HOME environment variable.\n");
					}
				}

			//change to directory specified in argument 
				else {
					printf("%s %s\n", args[0], args[1]);
					if (chdir(args[1])) {					//warning: passing argument 1 of 'chdir' from incompatible pointer type 
						perror("Cannot find directory.\n");
					}
				}
			else 
				printf("");
		}
		else if (strcmp(args[0], "status") == 0) {
			printf("STATUS!\n");

			//print exit status or
			if (WIFEXITED(0)) {
				int exitStatus = WEXITSTATUS(0);
			}

			//print terminating signal of last foreground process 
			else {
				int termSignal = WTERMSIG(0);
			}
		}

		//Check for input and output files	and store names
		/*
		if (inputIndex != -1 && inputIndex < numArgs - 1) {	//there is a '>' within bounds 
			inputFile = args[inputIndex + 1];
			printf("Input file: %s\n", inputFile);
		}
		if (outputIndex != -1 && outputIndex < numArgs - 1) {	//there is a '>' within bounds 
			outputFile = args[outputIndex + 1];
			printf("Output file: %s\n", outputFile);
		}
		*/


		//Run other commands with fork(), exec(), and waitpid()
		else {
			printf("Other shell commands!\n");
		}
	}

	return 0;

}		
