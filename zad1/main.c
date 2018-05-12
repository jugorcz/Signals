#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

pid_t childPid;
int sigIntReceived = 0;
int runningChildProcess = 0;
int createdProcesses = 0;

void sigStpHandler(int signum, siginfo_t* si, void* v) {
	if(runningChildProcess == 1) {
		printf("\nProces bedzie zakończony.\n");
		runningChildProcess = 0;
		kill(getpid()+createdProcesses, SIGKILL);
	}
	else {
		printf("\n\nProces bedzie utworzony.\n");
	}
}

void sigIntHandler(int signum) {
	if(runningChildProcess == 1) {
		printf("\nProces bedzie zakończony.\n");
		runningChildProcess = 0;
		kill(getpid()+createdProcesses, SIGKILL);
	}
	printf("A teraz cały program się zakończy.\n");
	exit(1);
}

int main(){

	struct sigaction sigact;
	sigact.sa_sigaction = sigStpHandler;
	sigact.sa_flags = SA_SIGINFO;
	sigaction(SIGTSTP, &sigact, NULL);

	signal(SIGINT, sigIntHandler);

	printf("proces rodzica: %d\n",getpid() );
	
	while(sigIntReceived == 0) {
		if(runningChildProcess == 0) {
			createdProcesses++;
			runningChildProcess = 1;
			pid_t childPid = fork();
			if(childPid == 0){
				printf("Tworze nowy proces\n");
				childPid = getpid();
				printf("proces dziecka: %d\n", childPid);
				execl("s1", "", NULL);
				return 0;
			} else pause();
		}pause();
	

	}
}