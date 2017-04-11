#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 4096
#define READ 0
#define WRITE 1

int main(void){
  int fd[2], a[2];
  char line[MAXLINE];
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
    fgets(line, MAXLINE, stdin);
    write(fd[WRITE], line, strlen(line)+1);
  } else {   /* child, reader */
    close(fd[WRITE]);
    read(fd[READ], line, MAXLINE);
    if(sscanf(line, "%d %d", &a[0], &a[1]) == 2){
      printf("Soma: %d\nDiferenca: %d\nMultiplicacao: %d\nDivisao: ", a[0]+a[1], a[0]-a[1], a[0]*a[1]);
      if(a[1]== 0)
        printf("Divisao por 0 - INVALIDA");
      else
        printf("%f", (double)a[0]/a[1]);
    } else
      printf("Nao eram 2 numeros");
  }
   exit(0);
}
