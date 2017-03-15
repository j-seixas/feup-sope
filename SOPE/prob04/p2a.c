// PROGRAMA p02a.c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
void sigint_handler(int signo)
{
  printf("Entering SIGINT handler ...\n");
  sleep(10);
  printf("Exiting SIGINT handler ...\n");
}

void sigterm_handler(int signo)
{
  printf("Entering SIGTERM handler ...\n");
  printf("Exiting SIGTERM handler ...\n");
}

int main(void)
{
 struct sigaction actionINT;
 actionINT.sa_handler = sigint_handler;
 sigemptyset(&actionINT.sa_mask);
 actionINT.sa_flags = 0;

 struct sigaction actionTERM;
 actionTERM.sa_handler = sigterm_handler;
 sigemptyset(&actionTERM.sa_mask);
 actionTERM.sa_flags = 0;


 if (sigaction(SIGINT,&actionINT,NULL) < 0) {
    fprintf(stderr,"Unable to install SIGINT handler\n");
    exit(1);
} else if(sigaction(SIGTERM,&actionTERM,NULL) < 0) {
    fprintf(stderr,"Unable to install SIGTERM handler\n");
    exit(1);
}
 printf("Try me with CTRL-C ...\n");
 while(1) pause();
 exit(0);
}
