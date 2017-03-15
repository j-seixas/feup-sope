// PROGRAMA p3a.c  NOT DONE!!!!
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int step = 1;

void sigusr_handler(int signo)
{
  if(signo == SIGUSR1)
    step = 1;
  else if(signo == SIGUSR2)
    step = -1;
}


int main(void)
{
 struct sigaction action;
 action.sa_handler = sigusr_handler;
 sigemptyset(&action.sa_mask);
 action.sa_flags = 0;



 if (sigaction(SIGUSR1,&action,NULL) < 0) {
    fprintf(stderr,"Unable to install SIGUSR1 handler\n");
    exit(1);
} else if(sigaction(SIGUSR2,&action,NULL) < 0) {
    fprintf(stderr,"Unable to install SIGUSR2 handler\n");
    exit(1);
}

int var = 0;
 while(1){
   printf("%d\n", var);
   sleep(1);
   var+=step;
 }
 exit(0);
}
