#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#define MAXLINE 4096
#define READ 0
#define WRITE 1

int main(void){
  int fd[2], a[2];
  pid_t pid;
  if (pipe(fd) < 0) {
    fprintf(stderr,"Pipe error");
    exit(1);
  }
  if ( (pid = fork()) < 0) {
    fprintf(stderr, "Fork error");
    exit(2);
  } else if (pid > 0) { /* parent, writer */
    close(fd[READ]);
    scanf("%d %d", &a[0], &a[1]);
    write(fd[WRITE], a, 2*sizeof(int));
  } else {   /* child, reader */
    close(fd[WRITE]);
    read(fd[READ], a, 2*sizeof(int));
    printf("Soma: %d\nDiferenca: %d\nMultiplicacao: %d\nDivisao: ", a[0]+a[1], a[0]-a[1], a[0]*a[1]);
    if(a[1] == 0)
      printf("Divisao por 0 - INVALIDA");
    else
      printf("%f", (double)a[0]/a[1]);
   }
   exit(0);
}
