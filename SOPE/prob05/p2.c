#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 4096
#define READ 0
#define WRITE 1

int main(void){
  int fd[2], p[2], a[2], result;
  double res;
  char line[MAXLINE], simb[2];
  pid_t pid;
  if (pipe(fd) < 0) {
    fprintf(stderr,"Pipe error");
    exit(1);
  }
  if (pipe(p) < 0) {
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
    close(p[WRITE]);
    read(p[READ], &result, sizeof(int));
    printf("Soma: %d\n", result);
    read(p[READ], &result, sizeof(int));
    printf("Diferenca: %d\n", result);
    read(p[READ], &result, sizeof(int));
    printf("Multiplicacao: %d\n", result);
    read(p[READ], simb, 2);
    if(simb[0] == 'x')
      printf("Divisao por 0 - INVALIDA");
    else{
      read(p[READ], &res, sizeof(double));
      printf("Divisao: %f", res);
    }

  } else {   /* child, reader */
    close(fd[WRITE]);
    read(fd[READ], line, MAXLINE);
    close(p[READ]);
    if(sscanf(line, "%d %d", &a[0], &a[1]) == 2){
      result = a[0]+a[1];
      write(p[WRITE], &result, sizeof(int));
      result = a[0]-a[1];
      write(p[WRITE], &result, sizeof(int));
      result = a[0]*a[1];
      write(p[WRITE], &result, sizeof(int));
      if(a[1]== 0) {
        simb[0] = 'x';
        write(p[WRITE], simb, strlen(simb)+1);
      } else {
        simb[0] = 'f';
        write(p[WRITE], simb, strlen(simb)+1);
        res = (double)a[0]/a[1];
        write(p[WRITE], &res, sizeof(double));
      }
    } else
      printf("Nao eram 2 numeros");




  }
   exit(0);
}
