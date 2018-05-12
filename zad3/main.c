#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

int signalsAmount;
int actionType;

int signalReceived = 1;
int sentSignals = 0;

int sentToChild = 0;
int gotByChild = 0;
int gotByParent = 0;

pid_t parentPid;
pid_t childPid;

int sendingFinished = 0;

int parseArguments(int argc, char**argv) 
{
	if(argc != 3) {
		printf("\nWrong arguments number, should be 2:\n");
		printf("	L - signals amount \n");
		printf("	T - type of action (1, 2 or 3)\n\n");
		return 0;
	}

	signalsAmount = (int)strtol(argv[1],NULL,10);
	if(signalsAmount == 0){
		printf("Wrong argument L!\n");
		return 0;
	}


	actionType = (int)strtol(argv[2],NULL,10);
	if(actionType < 1 || actionType > 3){
		printf("Wrong argument T!\n");
		return 0;
	}

	printf("Signals ammount: %d\n",signalsAmount );
	printf("Action type: ");
	if(actionType == 1) printf("Send signals with kill function\n");
	if(actionType == 2) printf("Send signals with kill function and wait for confirmation\n");
	if(actionType == 3) printf("Send run time signals with kill function\n");

	return 1;
}

void sendSignal(pid_t destPid, int signal) {
	if(destPid == childPid && signal != SIGUSR2) gotByChild++;
	if(actionType != 3) {
		kill(destPid, signal);
	}
  	else {
  		kill(destPid,SIGRTMIN + signal);
  	}
}

void handleUser1Signal(int signum, siginfo_t* si, void* v) {
	if(si -> si_pid == parentPid) { //signal from parent
		sendSignal(parentPid,SIGUSR1);
	}
	if(si -> si_pid == childPid) { //confirmation from chiild
		gotByParent++;
		signalReceived = 1;
	}
}

void handleUser2Signal(int signum) {
	sendingFinished = 1;
}

void handleIntSignal(int signum) {
	if(signum == SIGINT) {
		printf("SIGINT received.\n");
		kill(childPid, SIGKILL);
		exit(1);
	}
}

int main(int argc, char** argv) {
	
	if(parseArguments(argc,argv) == 0) return 0;
	parentPid = getpid();
	
	/* signals handling */

	//SIGUSR1
	struct sigaction sigact;
	sigact.sa_sigaction = handleUser1Signal;
	sigact.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sigact, NULL);

	//SIGRT as SIGUSR1
	sigaction(SIGRTMIN+SIGUSR1, &sigact, NULL);

	//SIGUSR2
	signal(SIGUSR2, handleUser2Signal);

	//SIGRT as SIGUSR2
	signal(SIGRTMIN+SIGUSR2, handleUser2Signal);

	//SIGINT
	signal(SIGINT, handleIntSignal);

	/* run child process */
	childPid = fork();
	if(childPid == 0) {//child process
		while(sendingFinished == 0) {
			pause();
		}
		return 0;
	} 
	else if(childPid > 0) {//parent process
		for(int i = 0; i < signalsAmount; i++) {
			sendSignal(childPid, SIGUSR1);
			signalReceived = 0;
			usleep(25);
			sentSignals++;

			if(actionType == 2) {
				while(signalReceived == 0) {
					pause();
				}
			}
		}
		
		sendSignal(childPid,SIGUSR2);
		int status;
		wait(&status);
		printf("\nSent to child: %d", sentSignals);
		printf("\nGot by child: %d", gotByChild);
		printf("\nGot by parent: %d\n", gotByParent);
	}
	return 0;
}