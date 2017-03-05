/*
** smallsh.c  
** Morgan Brenner
** CS 344 Program 3 
** 03/05/2017 
*/

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

int foregroundOnly = 0;							//for SIGTSTP 

void catchSIGINT(int signo);
void catchSIGTSTP(int signo);

int main() {
	int shellStatus = 0;
	char commandLine[MAX_CHAR];						
	char *ptr, *inputFile, *outputFile;				
	int i, j;
	int numArgs, inputIndex, outputIndex, pidIndex;	//track special characters 
	int pid, pidLen, lenWithPid;					//for replacing '$$' with pid
	char *args[MAX_ARGS];
	char *filteredArgs[MAX_ARGS];					//holds arguments minus '&' 
	char pidBuffer[PID_BUFFER_SIZE];
	char* argWithPid;							

	int sourceFD, targetFD, inputResult, outputResult;
	char *ioArg[1];
	pid_t childPid;	

	bool isBackgroundProcess; 
	int numBgProcesses = 0;
	int bgProcesses[MAX_ARGS];					//store PIDs of non-completed processes 

	//initialize sigaction structs, and block actions  
	struct sigaction SIGINT_action = { 0 }, SIGTSTP_action = { 0 };

	//initialize signal handlers 
	SIGINT_action.sa_handler = catchSIGINT;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0; 

	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;

	//assign behaviors to override default actions 
	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	while (1) {	
		numArgs = 0; 
		inputIndex = outputIndex = pidIndex = -1;
		isBackgroundProcess = false;			//foreground process by default 

		printf(": ");						//use ': ' as prompt for each command line 
		fflush(stdout);	fflush(stdin);		//flush input & output buffers immediately after each output 

		fgets(commandLine, MAX_CHAR, stdin);
		if (commandLine != NULL)
			ptr = strtok(commandLine, " \n");			
 
		if (ptr != NULL && ptr[0] != '#') {	//if line is not comment 
			while (ptr != NULL) {
				// store index of input/output redirection symbols
				if (strcmp(ptr, "<") == 0) {				
					inputIndex = numArgs;
				}
				else if (strcmp(ptr, ">") == 0) {			
					outputIndex = numArgs;
				}

				// detect '$$' within string and expand $$ to pid of 
				// shell (should work when adjacent to quotes like in 'echo "PID: $$"')
				if (strlen(ptr) > 1) {					//string length must be > 1 to contain "$$" 
					for (i = 1; i < strlen(ptr); i++) {
						if ((ptr[i - 1] == '$') && (ptr[i] == '$')) {
							pidIndex = i - 1; 
							break;
						}		
					}
				}
				if (pidIndex >= 0) {
					pid = getpid();
					snprintf(pidBuffer, PID_BUFFER_SIZE, "%d", pid);
					pidLen = strlen(pidBuffer);						//get length of pid as string
					lenWithPid = strlen(pidBuffer) + strlen(ptr) - 2;		//new length with pid replacing $$ 

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
 
			args[numArgs] = ptr;				//add last argument to args 
			
			//Check for built-in commands
			if (strcmp(args[0], "exit") == 0) {
				//kill remaining processes 
				if (numBgProcesses > 0) {
					for (i = 0; i < numBgProcesses; i++) {
						kill(bgProcesses[i], SIGKILL);
						printf("Bg process terminated\n");
						fflush(stdout);
					}	
				}
					exit(0);
			}

			else if (strcmp(args[0], "cd") == 0) {
				if (numArgs == 1 || numArgs == 2)
					//change to directory in HOME environment variable 
					if (numArgs == 1) {
						if (chdir(getenv("HOME"))) {
							perror("Cannot find HOME environment variable.\n");
						}
					}

				//change to directory specified in argument 
					else {
						if (chdir(args[1])) {					
							perror("Cannot find directory.\n");
						}
					}
			}
			else if (strcmp(args[0], "status") == 0) {
				//print exit status or...
				if (WIFEXITED(shellStatus)) {
					printf("exit value %d\n", WEXITSTATUS(shellStatus));
					fflush(stdout);
				}

				//print terminating signal of last foreground process 
				else {
					printf("terminated by signal %d\n", WTERMSIG(shellStatus));
					fflush(stdout);
				}
				
			}
		

			//Run other commands with fork(), exec(), and waitpid()
			else {
				//check for background process - & at end of args 
				if (strcmp(args[numArgs - 1], "&") == 0) {
					isBackgroundProcess = true; 
					
					for (i = 0; i < numArgs - 1; i++) {
						filteredArgs[i] = args[i];
					}					 
					numArgs--;
				}

				//start child process
				pid_t childPid = -5;
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
						sourceFD = open(inputFile, O_RDONLY);							
						if (sourceFD == -1) { 
							printf("cannot open %s for input\n", inputFile); 
							fflush(stdout);
							shellStatus = 1; 
							exit(1);
						}
						else {
							inputResult = dup2(sourceFD, 0);
							if (inputResult == -1) { perror("dup2()"); shellStatus = 1; }
							close(sourceFD);
						}
					}

					if (outputIndex >= 0 && outputIndex < numArgs - 1) {	//there is a '>' within bounds 
						outputFile = args[outputIndex + 1];
						targetFD = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);			
						if (targetFD == -1) { perror("target open()"); shellStatus = 1; }
						outputResult = dup2(targetFD, 1);								
						if (outputResult == -1) { perror("dup2()"); shellStatus = 1; }
						close(targetFD);		
					}

					//for bg processes: if no input specified, redirect from dev/null
					if (foregroundOnly == 0) {
						if (isBackgroundProcess == true && inputFile != NULL) {
							inputFile = "/dev/null";
							sourceFD = open(inputFile, O_RDONLY);
							if (sourceFD == -1) { perror("source open()"); shellStatus = 1; }   //error here 
							inputResult = dup2(sourceFD, 0);
							if (inputResult == -1) { perror("dup2()"); shellStatus = 1; }	//error here 
							close(sourceFD);
						}

						if (isBackgroundProcess == true && outputFile != NULL) {
							outputFile = "/dev/null";
							targetFD = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
							if (targetFD == -1) { perror("target open()"); shellStatus = 1; }
							outputResult = dup2(targetFD, 1);
							if (outputResult == -1) { perror("dup2()"); shellStatus = 1; }
							close(targetFD);
						}
					}
				    
					//new process to execute command
					//execute bg process 
					if (isBackgroundProcess == true) {
						execvp(filteredArgs[0], filteredArgs);
						printf("Could not find command\n");
						fflush(stdout);
						exit(1);
					}
					//execute command with i/o arguments 
					if ((inputIndex >= 0) || (outputIndex >= 0)) {
						ioArg[0] = args[0];				//only take 1st argument (ignore args after > and <)
						execvp(args[0], ioArg);
						printf("%s: no such file or directory\n", args[0]);
						fflush(stdout);
						exit(1);	
					}
					//execute normal command 
					else {
						execvp(args[0], args);
						printf("%s: no such file or directory\n", args[0]);
						fflush(stdout);	
						exit(1);
					}
				}										//else if (childPid == 0)
				//immediately wait for foreground processes
				if (isBackgroundProcess == false || foregroundOnly != 0) {
					waitpid(childPid, &shellStatus, 0);
				}
				//add background processes to queue 
				else {
					printf("background pid is %d\n", childPid);
					fflush(stdout);

					bgProcesses[numBgProcesses] = childPid;
					numBgProcesses++;
					//printf("added bg process #%d: %d\n", numBgProcesses, childPid);
					fflush(stdout);
				}

				if (WIFEXITED(shellStatus)) {
					WEXITSTATUS(shellStatus);		//killed by exit
				}
				else {
					WTERMSIG(shellStatus);			//killed by signal 
				}
			}										//if other command 
		}											//if not comment

		//check for terminating bg processe before next prompt 
		childPid = waitpid(-1, &shellStatus, WNOHANG);	//look for a child 
		while (childPid > 0) {						//a child still exists 
			//if exit
			if (WIFEXITED(shellStatus)) {
				printf("background pid %d is done: exit value %d\n", childPid, WEXITSTATUS(shellStatus));
				fflush(stdout);
				numBgProcesses--;
			}
			//if signal
			else if (WIFSIGNALED(shellStatus)) {
				printf("background pid %d is done: terminated by signal %d\n", childPid, WTERMSIG(shellStatus));
				fflush(stdout);
				numBgProcesses--;
			}
			childPid = waitpid(-1, &shellStatus, WNOHANG);	//look for next child 
		}

	}												//main while loop					
	if (argWithPid != NULL)
		free(argWithPid);

	return 0;
}	

//^C will kill foreground process
void catchSIGINT(int signo) {
	//foreground signal terminates self
	puts("\nForeground signal terminating.\n");				//cannot use printf in signal handlers
	fflush(stdout);
	//kill child process 
	waitpid(signo);
}

//^Z will toggle foreground-only mode  
void catchSIGTSTP(int signo) {
	if (foregroundOnly == 0) {
		puts("\nEntering foreground-only mode (& is now ignored)\n");	//cannot use printf in signal handlers
		fflush(stdout);
		foregroundOnly = 1; 
	}

	else {
		puts("\nExiting foreground-only mode\n");				//cannot use printf in signal handlers
		fflush(stdout);
		foregroundOnly = 0; 
	}
	
}
