#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

//execvp() example
void execute(char** argv) {
	if(execvp(*argv, argv) < 0) {
		perror("Exec failure!");
		exit(1);
	}
}

//using pointers to functions
int AddOne(int inputArg);

//catching & ignoring sinals 
void catchSIGINT(int signo) {
	char* message = "Caught SIGINT, sleeping for 5 seconds\n";
	write(STDOUT_FILENO, message, 38);	
	raise(SIGUSR2);
	sleep(5);
}

void catchSIGUSR2(int signo) {
	char* message = "Caught SIGUSR2, exiting!\n";
	write(STDOUT_FILENO, message, 25);
	exit(0);
}


void main() {
	pid_t spawnPid = -5;
	int childExitMethod = -5; 
	
	spawnPid = fork();
	if (spawnPid == -1) {
		perror("Hull breach!\n");
		exit(1);
	}
	else if (spawnPid == 0) {
		printf("CHILD: PID: %d, exiting!\n", spawnPid);
		exit(0);
	}
	
	printf("PARENT: PID: %d, waiting...\n", spawnPid);
	waitpid(spawnPid, &childExitMethod, 0);
	printf("PARENT: Child process terminated, exiting!\n");
	
	//check exit status 
	if (WIFEXITED(childExitMethod)) {
		printf("The process exited normally\n");
		int exitStatus = WEXITSTATUS(childExitMethod);
		printf("exit status was %d\n", exitStatus);
	}
	else
		printf("Child terminated by signal\n");
	
	//fork() + execlp() example 
	/*
	spawnPid = -5; 
	childExitMethod = -5; 
	spawnPid = fork();
	
	switch(spawnPid) {
			case -1: { perror("Hull breach!\n"); exit(1); break; }
			case 0: {
				printf("CHILD (%d): Sleeping for 1 second\n", getpid());
				sleep(1);
				printf("CHILD (%d): Converting into \'ls -a\'\n", getpid());
				execlp("ls", "ls", "-a", NULL);
				perror("CHILD: exe failure!\n");
				exit(2); break;
			}
			default: {
				printf("PARENT (%d): Sleeping for 2 seconds\n", getpid());
				sleep(2);
				printf("PARENT (%d): Wait()ing for child (%d) to terminate\n", getpid(), spawnPid);
				pid_t actualPid = waitpid(spawnPid, &childExitMethod, 0);
				printf("PARENT (%d): Child (%d) terminated, exiting!\n", getpid(), actualPid);
				exit(0); break;
				
			}
	}
	*/
	
	//execvp example 
	/*
	char* args[3] = { "ls", "-a", NULL };
	printf("Replacing process with %s %s\n", args[0], args[1]);
	execute(args);
	*/
	
	//using pointers to functions
	int (*fpArg)(int) = AddOne;
	printf("10 + 1 = %d\n", fpArg(10));
	
	//catching & ignoring sinals 
	struct sigaction SIGINT_action = {0}, SIGUSR2_action = {0}, ignore_action = {0}; 
	
	SIGINT_action.sa_handler - catchSIGINT; 	//function defined above main
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
	
	SIGUSR2_action.sa_handler = catchSIGUSR2; 	//function defined above main 
	sigfillset(&SIGUSR2_action.sa_mask);
	SIGUSR2_action.sa_flags = 0; 
	
	ignore_action.sa_handler = SIG_IGN; 
	
	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGUSR2, &SIGUSR2_action, NULL);
	sigaction(SIGTERM, &ignore_action, NULL);	//ignore terminate, hangup, and quit signals  
	sigaction(SIGHUP, &ignore_action, NULL);
	sigaction(SIGQUIT, &ignore_action, NULL);
	
	printf("SIGTERM, SIGHUP, and SIGQUIT are disabled.\n");
	printf("Send a SIGUSR2 signal to kill this program.\n");
	printf("Send a SIGINT signal to sleep 5 seconds, then kill this program.\n");		// ^C
	
	while(1) {
		pause();		//sleep, but wake up to signals 
	}
	
	
	exit(0);
}

//using pointers to functions
int AddOne(int input) { return input + 1; }