#include <sys/types.h>		//pid_t, etc. 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CHAR 2048
#define MAX_ARGS 512
#define PID_BUFFER_SIZE 50

int main() {
	char commandLine[MAX_CHAR];
	char *ptr, *inputFile, *outputFile; 
	int i, j;
	int numArgs;
	int inputIndex, outputIndex, pidIndex; 
	int pid, pidLen, lenWithPid;
	char *args[MAX_ARGS];
	char pidBuffer[PID_BUFFER_SIZE];
	char* argWithPid; 

	bool backgroundProcess; 
	int numBgProcesses = 0;
	int bgProcesses[MAX_ARGS];		//store PIDs of non-completed processes 

	while (1) {				    
		numArgs = 0; 
		inputIndex = outputIndex = pidIndex = -1;
		backgroundProcess = false;

		printf(": ");						//use ': ' as prompt for each command line 
		fflush(stdout);	fflush(stdin);		//flush input & output buffers immediately after each output 

		fgets(commandLine, MAX_CHAR, stdin);
		ptr = strtok(commandLine, " \n");			

		if (strcmp(ptr, "#") != 0) {					//if not comment 
			while (ptr != NULL) {


				if (strcmp(ptr, ">") == 0) {			//store index of input redirection symbol	
					printf("input!\n");
					inputIndex = numArgs;
				}

				else if (strcmp(ptr, "<") == 0) {		//store index of output redirection symbol	
					printf("output!\n");
					outputIndex = numArgs;
				}

				//detect '$$' within string
				if (strlen(ptr) > 1) {				//string length must be > 1 to contain "$$" 
					for (i = 1; i < strlen(ptr); i++) {
						if ((ptr[i - 1] == '$') && (ptr[i] == '$')) {
							printf("Going to expand $$ to pid!!\n");
							pidIndex = i - 1; 
							break;
						}		
					}
				}

				//expand $$ to pid of shell  (should work with 'echo "PID: $$"')	
				if (pidIndex >= 0) {
					pid = getpid();
					snprintf(pidBuffer, PID_BUFFER_SIZE, "%d", pid);
					pidLen = strlen(pidBuffer);						//get length of pid as string
					printf("Pid %d has length %d\n", pid, pidLen);

					lenWithPid = strlen(pidBuffer) + strlen(ptr) - 2;		//new length with pid replacing $$ 
					printf("New string length: %d\n", lenWithPid);

					argWithPid = malloc(lenWithPid * sizeof(char));		//maybe store these in an array for cleanup?

					for (i = 0; i < pidIndex; i++) 
						argWithPid[i] = ptr[i];						//copy part of string preceding $$ 
					
					j = 0;
					for (i = pidIndex; i <= pidLen; i++) {
						argWithPid[i] = pidBuffer[j++];				//copy pid as string into arg string
					}

					if (pidIndex + pidLen < lenWithPid) {				//not end of string
						j = pidIndex + 2; 
						for (i = pidIndex + pidLen; i < lenWithPid; i++)
							argWithPid[i] = ptr[j++];				//copy part of string following $$ if applicable 
					}

					ptr = argWithPid;								//reassign ptr to updated string
				}


				args[numArgs] = ptr;			//store command/argument in array 
				ptr = strtok(NULL, " \n");
				numArgs++;
			}								//while ptr != null
 
			args[numArgs] = ptr;					//last argument 

			printf("# Arguments: %d\n", numArgs);
			for (i = 0; i < numArgs; i++) {
				printf("%s ", args[i]);
			}
			printf("\n");
			
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
					int dummy_pid = getpid();				//placeholder for testing 
					bgProcesses[numBgProcesses] = dummy_pid;
					numBgProcesses++;

					//test print
					printf("Background processes:\n");
					for (i = 0; i < numBgProcesses; i++) {
						printf("%d ", bgProcesses[i]);
					}
					printf("\n");
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
			}										//if other command 
		}											//if not comment
	}												//main while loop					


	return 0;

}		
