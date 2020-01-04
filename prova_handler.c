#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>


void handler(int signal, siginfo_t *si, void * ucontext);

int main(){
    printf("pid %d",getpid());
    
    union sigval sv; 
    struct sigaction sa;
    bzero(&sa,sizeof(sa));
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);
    sv.sival_int = 43;
    sigqueue(getpid(),SIGUSR1,sv);

}

void handler(int signal, siginfo_t *si, void * ucontext){
    printf("int %d \n",si->si_value);
    printf("int %d \n",si->si_pid);

}


