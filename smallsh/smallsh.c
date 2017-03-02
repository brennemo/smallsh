#include <sys/types.h>		//pid_t, etc. 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CHAR 2048
#define MAX_ARGS 512

int main() {
	char commandLine[MAX_CHAR];
	char *ptr, *inputFile, *outputFile; 
	int i, numArgs, inputIndex, outputIndex;
	char *args[MAX_ARGS];

	//bool inputRedirect, outputRedirect, backgroundProcess; 
	bool backgroundProcess; 
	int numBgProcesses = 0;
	int bgProcesses[MAX_ARGS];		//store PIDs of non-completed processes 

	while (1) {				    
		numArgs = 0; 
		inputIndex = outputIndex = -1;
		//inputRedirect = outputRedirect = backgroundProcess = false;
		backgroundProcess = false;

		printf(": ");						//use ': ' as prompt for each command line 
		fflush(stdout);	fflush(stdin);		//flush input & output buffers immediately after each output 

		fgets(commandLine, MAX_CHAR, stdin);
		
		ptr = strtok(commandLine, " \n");			
		//printf("command: %s\n", ptr);
		if (strcmp(ptr, "#") != 0) {					//if not comment 
			while (ptr != NULL) {

				//ignore comments beginning with '#'
				/*if (strcmp(ptr, "#") == 0) {
					continue;
				}	*/

				if (strcmp(ptr, ">") == 0) {			//store index of input redirection symbol	
					//printf("input!\n");
					inputIndex = numArgs;
				}

				else if (strcmp(ptr, "<") == 0) {		//store index of output redirection symbol	
					//printf("output!\n");
					outputIndex = numArgs;
				}


				else if (strcmp(ptr, "$") == 0) {		//expand $$ to pid of shell  		
					printf("Going to expand $$ to pid!!\n");
				}



				args[numArgs] = ptr;			//store command/argument in array 
				ptr = strtok(NULL, " \n");
				numArgs++;
			}
 
			args[numArgs] = ptr;					//last argument 

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
					exit(0);
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
						if (chdir(args[1])) {					
							perror("Cannot find directory.\n");
						}
					}
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
		

			//Run other commands with fork(), exec(), and waitpid()
			else {
				printf("Other shell commands!\n");

				//check for background process - & at end of args 
				if (strcmp(args[numArgs - 1], "&") == 0) {
					printf("Background process!\n");
					int dummy_pid = 1234;				//placeholder for testing 
					bgProcesses[numBgProcesses] = dummy_pid;
					numBgProcesses++;
				}

				//spawn child process for i/o redirection
				printf("Creating child process\n");

				//use dup2 to set up redirection

				//Check for input and output files	and store names

				if (inputIndex >= 0 && inputIndex < numArgs - 1) {	//there is a '>' within bounds 
					inputFile = args[inputIndex + 1];
					printf("Input file: %s\n", inputFile);
				}
				if (outputIndex >= 0 && outputIndex < numArgs - 1) {	//there is a '>' within bounds 
					outputFile = args[outputIndex + 1];
					printf("Output file: %s\n", outputFile);
				}

				//Remove symbol and file name from arguments list 
				if (inputIndex >= 0 && inputIndex < numArgs - 1) {
					for (i = inputIndex; i < numArgs - 2; i++) {
						args[i] = args[i + 2];					//remove redirection symbol & filename 
					}
				
					numArgs -= 2;	outputIndex -= 2;				//adjust indices 
				}

				if (outputIndex >= 0 && outputIndex < numArgs - 1) {
					for (i = outputIndex; i < numArgs - 2; i++) 
						args[i] = args[i + 2];					//remove redirection symbol & filename 	
				
					numArgs -= 2;
				}

				//printf("Redirecting stdin\n");

				//printf("Redirecting stdout\n");

				//exec command 
				printf("\n");
					//expand $$ into process ID of shell itself 

					//look for non-built in commands in PATH variable 
				
					//command not found - error message & set exit status to 1 

				//clean up 
			}
		}
	}									//if not comment


	return 0;

}		
