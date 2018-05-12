#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

int processAmmount;
int requestsLimit;
int currentRequestsNumber;

pid_t* childrenPids;

int parseArguments(int argc, char**argv) 
{
	if(argc != 3) {
		printf("\nWrong arguments number, should be 2:\n");
		printf("	N - child process ammount \n");
		printf("	M - access requests limit (M<N)\n\n");
		return 0;
	}

	processAmmount = (int)strtol(argv[1],NULL,10);
	if(processAmmount == 0){
		printf("Wrong argument N!\n");
		return 0;
	}


	requestsLimit = (int)strtol(argv[2],NULL,10);
	if(requestsLimit == 0 || requestsLimit > processAmmount){
		printf("Wrong argument M!\n");
		return 0;
	}

	printf("Process ammount: %d\n",processAmmount );
	printf("Requests limit: %d\n",requestsLimit);

	return 1;
}

void handlerSIGUSER1(int signum, siginfo_t* si, void* v) {
	//printf("\n%d odbiera sygnał od %d\n",getpid(),si->si_pid);
	currentRequestsNumber++;
	if(currentRequestsNumber < requestsLimit) { //add process pid to waiting queue 
		childrenPids[currentRequestsNumber-1] = si -> si_pid;
	} else {
		if(currentRequestsNumber == requestsLimit) { //wake up all waiting processes 
			//printf("%d budzi procesy: ",getpid());
			for(int i = 0; i < currentRequestsNumber; i++) {
				//printf("\n -> %d wakes up",(int)childrenPids[i]);
				kill(childrenPids[i], SIGALRM);
			}
		} else { //wake up only this process
			//printf("\n --> %d wakes up",(int)si->si_pid);
			kill(si -> si_pid, SIGALRM);
		}
	}
}

void handlerSIGALRM(int signum) {/*printf("SIGALARM!!!\n"); */}

void handlerSIGRunTime(int signum, siginfo_t* si, void* v)
{
    if(signum == SIGRTMIN)
        printf("\nSIGRTMIN received, pid : %d", si->si_pid);
    else
    {
        if(signum == SIGRTMAX)
            printf("\nSIGRTMAX received, pid : %d", si->si_pid);
        else
            printf("\n%d received, pid : %d", signum, si->si_pid);
    } 
}

void handlerSIGINT(int signum)
{
    pid_t parentPid = getpid();
    currentRequestsNumber = processAmmount;
    sigset_t signal_set;
    sigemptyset (&signal_set);
    sigaddset(&signal_set,SIGTERM);
    sigset_t old_set;
    sigprocmask(SIG_BLOCK, &signal_set, &old_set);
    kill(-1*parentPid, SIGTERM);
    sigprocmask(SIG_SETMASK, &old_set, NULL);
}

int main(int argc, char** argv) {
	if(parseArguments(argc,argv) == 0) return 0;

	/* signals handling */

	struct sigaction sigact;
	sigact.sa_sigaction = handlerSIGUSER1;
	/* If SA_SIGINFO is specified in sa_flags, then sa_sigaction (instead of
       sa_handler) specifies the signal-handling function for signum. */
	sigact.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sigact, NULL);

	signal(SIGALRM, handlerSIGALRM); //wake up from waiting for permission

	sigact.sa_sigaction = handlerSIGRunTime;//for runtime signals
	for(int i = SIGRTMIN; i <= SIGRTMAX; i++)
        sigaction(i, &sigact, NULL);

    signal(SIGINT, handlerSIGINT);

	/* run child process */
	currentRequestsNumber = 0;
	childrenPids = (pid_t*)malloc(processAmmount * sizeof(pid_t));

	for(int i = 0; i < processAmmount; i++) {
		int signalNumber = rand() % (SIGRTMAX - SIGRTMIN + 1) + SIGRTMIN;
		int sleepTime = rand() % 11;
		pid_t pid = fork();

		if(pid == 0) {
			//printf("%d start\n",getpid());
			sleep(sleepTime);
			pid_t parentPid = getppid();
			//printf("%d wysyła request\n",getpid());
			kill(parentPid, SIGUSR1);
			pause();
			//sleep(256); //wait for permission
			//printf("%d wysyła sygnał %d\n",getpid(),signalNumber);
			kill(parentPid, signalNumber);
			return sleepTime;
		}
	}

	while(1) {
		if(currentRequestsNumber >= processAmmount) return 0;
		int status; //wait() store status information
		pid_t pid = wait(&status);  //returns the process ID of the terminated child
		if(pid != -1) 
			printf("\nProcess ended : %d, exit status : %d", pid, status);
	}

	return 0;
}
