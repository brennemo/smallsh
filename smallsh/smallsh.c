#include <sys/types.h>		//pid_t, etc. 
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CHAR 2048
#define MAX_ARGS 512
#define PID_BUFFER_SIZE 50

void catchSIGINT(int signo);
void catchSIGTSTP(int signo);

int main() {
	int shellStatus = 0;
	char commandLine[MAX_CHAR];						
	char *ptr, *inputFile, *outputFile;				
	int i, j;
	int numArgs, inputIndex, outputIndex, pidIndex; 
	int pid, pidLen, lenWithPid;
	char *args[MAX_ARGS];
	char pidBuffer[PID_BUFFER_SIZE];
	char* argWithPid; 

	int sourceFD, targetFD, inputResult, outputResult;
	pid_t childPid;	
	int childExitMethod;

	bool backgroundProcess; 
	int numBgProcesses = 0;
	int bgProcesses[MAX_ARGS];		//store PIDs of non-completed processes 

	//initialize sigaction structs, and block actions  
	struct sigaction SIGINT_action = { 0 }, SIGTSTP_action = { 0 };

	SIGINT_action.sa_handler = catchSIGINT;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0; 

	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;

	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	while (1) {				    
		numArgs = 0; 
		inputIndex = outputIndex = pidIndex = -1;
		backgroundProcess = false;

		printf(": ");						//use ': ' as prompt for each command line 
		fflush(stdout);	fflush(stdin);		//flush input & output buffers immediately after each output 

		fgets(commandLine, MAX_CHAR, stdin);
		if (commandLine != NULL)
			ptr = strtok(commandLine, " \n");			

		if (ptr != NULL && strcmp(ptr, "#") != 0) {					//if line is not comment 
			while (ptr != NULL) {


				/*
				** store index of input/output redirection symbols
				*/
				if (strcmp(ptr, "<") == 0) {				
					//printf("input!\n");
					inputIndex = numArgs;
				}

				else if (strcmp(ptr, ">") == 0) {			
					//printf("output!\n");
					outputIndex = numArgs;
				}

				/*
				** detect '$$' within string and expand $$ to pid of 
				** shell (should work when adjacent to quotes like in 'echo "PID: $$"')
				*/
				
				if (strlen(ptr) > 1) {				//string length must be > 1 to contain "$$" 
					for (i = 1; i < strlen(ptr); i++) {
						if ((ptr[i - 1] == '$') && (ptr[i] == '$')) {
							//printf("Going to expand $$ to pid!!\n");
							pidIndex = i - 1; 
							break;
						}		
					}
				}
				if (pidIndex >= 0) {
					pid = getpid();
					snprintf(pidBuffer, PID_BUFFER_SIZE, "%d", pid);
					pidLen = strlen(pidBuffer);						//get length of pid as string
					//printf("Pid %d has length %d\n", pid, pidLen);

					lenWithPid = strlen(pidBuffer) + strlen(ptr) - 2;		//new length with pid replacing $$ 
					//printf("New string length: %d\n", lenWithPid);

					argWithPid = malloc(lenWithPid * sizeof(char));		
					memset(argWithPid, '\0', lenWithPid);

					for (i = 0; i < pidIndex; i++) 
						argWithPid[i] = ptr[i];						//copy part of string preceding $$ 
					
					j = 0;
					for (i = pidIndex; i <= pidLen; i++) 
						argWithPid[i] = pidBuffer[j++];				//copy pid as string into arg string

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

			/*
			printf("# Arguments: %d\n", numArgs);
			for (i = 0; i < numArgs; i++) {
				printf("%s ", args[i]);
			}
			printf("\n");
			*/
			
			//Check for built-in commands
			if (strcmp(args[0], "exit") == 0) {
					exit(0);
			}

			else if (strcmp(args[0], "cd") == 0) {
				if (numArgs == 1 || numArgs == 2)
					//change to directory in HOME environment variable 
					if (numArgs == 1) {
						//printf("%s\n", args[0]);
						if (chdir(getenv("HOME"))) {
							perror("Cannot find HOME environment variable.\n");
						}
					}

				//change to directory specified in argument 
					else {
						//printf("%s %s\n", args[0], args[1]);
						if (chdir(args[1])) {					
							perror("Cannot find directory.\n");
						}
					}
			}
			else if (strcmp(args[0], "status") == 0) {
				//print exit status or...
				if (WIFEXITED(0)) {
					printf("exit value %d\n", WEXITSTATUS(shellStatus));
				}

				//print terminating signal of last foreground process 
				else {
					printf("terminated by signal %d\n", WTERMSIG(shellStatus));
				}
				
			}
		

			//Run other commands with fork(), exec(), and waitpid()
			else {
				//printf("Other shell commands!\n");

				//check for background process - & at end of args 
				if (strcmp(args[numArgs - 1], "&") == 0) {
					//printf("Background process!\n");
					int dummy_pid = getpid();				//placeholder for testing 
					bgProcesses[numBgProcesses] = dummy_pid;
					numBgProcesses++;

					//test print
					/*
					printf("Background processes:\n");
					for (i = 0; i < numBgProcesses; i++) {
						printf("%d ", bgProcesses[i]);
					}
					printf("\n");
					*/
				}

				/*
				if (outputIndex >= 0 && outputIndex < numArgs - 1) {	//there is a '>' within bounds 
					outputFile = args[outputIndex + 1];
					printf("Output file: %s\n", outputFile);

					for (i = outputIndex; i < numArgs - 2; i++) {
						args[i] = args[i + 2];					//remove redirection symbol & filename
					}

					printf("After shifting:\n");
					for (i = outputIndex; i < numArgs; i++) {
						printf("%s\n");
					}
					printf("\n");
				}
				*/

				//child process
				pid_t childPid = -5;	int childExitMethod = -5;
				childPid = fork();

				if (childPid == -1) {
					perror("Hull breach!\n");
					exit(1);
				}
				else if (childPid == 0) {
					//use dup2 to set up redirection
					//Check for input and output files	and store names
					if (inputIndex >= 0 && inputIndex < numArgs - 1) {	//there is a '<' within bounds 
						inputFile = args[inputIndex + 1];
						//printf("Input file: %s\n", inputFile);

						sourceFD = open(inputFile, O_RDONLY);							
						//printf("sourceFD = open(inputFile, O_RDONLY);\n");
						if (sourceFD == -1) { perror("source open()"); exit(1); }
						inputResult = dup2(sourceFD, 0);						//ERROR HERE
						//printf("inputResult = dup2(sourceFD, 0);\n");
						if (inputResult == -1) { perror("dup2()"); exit(1); }
						//printf("if (inputResult == -1) { perror(\"target open()\"); exit(1); }\n");
						close(sourceFD);		//warning: expected int, is char*
						//printf("close(sourceFD);\n");

						
						//printf("TEST!!!\n");		//not reaching here 

						for (i = inputIndex; i < numArgs - 2; i++) {
							args[i] = args[i + 2];					//remove redirection symbol & filename 
						}
						numArgs -= 2;	outputIndex -= 2;				//adjust indices 
						
					}

					if (outputIndex >= 0 && outputIndex < numArgs - 1) {	//there is a '>' within bounds 
						outputFile = args[outputIndex + 1];
						//printf("Output file: %s\n", outputFile);

						targetFD = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);			
						//printf("targetFD = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);\n");
						if (targetFD == -1) { perror("target open()"); exit(1); }
						outputResult = dup2(targetFD, 1);								//ERROR HERE
						//printf("outputResult = dup2(targetFD, 1);\n");
						if (outputResult == -1) { perror("dup2()"); exit(1); }
						//printf("if (outputResult == -1) { perror(\"source open()\"); exit(1); }\n");
						
						for (i = outputIndex; i < numArgs; i++)
							args[i] = args[i + 2];					//remove redirection symbol & filename

						//printf("inside if:\n");
						/*for (i = 0; i < numArgs; i++) {
							printf("%s ", args[i]);
						}
						printf("\n");	*/
						
						
						close(targetFD);			//warning: expected int, is char*
						//printf("close(targetFD);\n");

						
						//printf("TEST!!!\n");		
						

					
					}
					/*
					//Remove symbol and file name from arguments list 
					//if (inputIndex >= 0 && inputIndex < numArgs - 1) {
					if (inputFile != NULL) {
						for (i = inputIndex; i < numArgs - 2; i++) {
							args[i] = args[i + 2];					//remove redirection symbol & filename 
						}
						numArgs -= 2;	outputIndex -= 2;				//adjust indices 
					
					}


					//if (outputIndex >= 0 && outputIndex < numArgs - 1) {
					if (outputFile != NULL) {
						for (i = outputIndex; i < numArgs - 2; i++)
							args[i] = args[i + 2];					//remove redirection symbol & filename 	
					}
					*/

					//test print 
					/*
					printf("outside of if:\n");
					for (i = 0; i < numArgs; i++) {
						printf("%s ", args[i]);
					}
					printf("\n");													 
					*/


				    
					//new process to execute command
					if (execvp(args[0], args) < 0) {
						perror("Could not find command.");
						shellStatus = 1;		//?
						exit(1);
					}
					

					//printf("CHILD: PID: %d, exiting!\n", childPid);
					exit(0);
				}

				//printf("PARENT: PID: %d, waiting...\n", childPid);
				waitpid(childPid, &childExitMethod, 0);
				//printf("PARENT: Child process terminated, exiting!\n");

				if (WIFEXITED(childExitMethod)) {
					//printf("The process exited normally\n");
					int exitStatus = WEXITSTATUS(childExitMethod);
					//printf("exit status was %d\n", exitStatus);
				}
				//else
					//printf("Child terminated by signal\n");

				//clean up 
			}										//if other command 
		}											//if not comment
		
	}												//main while loop					
	if (argWithPid != NULL)
		free(argWithPid);

	return 0;
}	

void catchSIGINT(int signo) {
	//foreground signal terminates self
	//printf("\nForeground signal terminating.\n");
}

void catchSIGTSTP(int signo) {
	//printf("\nEntering foreground-only mode (& is now ignored)\n");

	//printf("\nExiting foreground-only mode\n");
}
