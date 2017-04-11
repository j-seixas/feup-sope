#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#define MAXLINE 4096
#define READ 0
#define WRITE 1

int main(void){
  int fd[2], a[2];
  struct {
    int n1, n2;
  } numbers;
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
    scanf("%d %d", &numbers.n1, &numbers.n2);
    write(fd[WRITE], &numbers, 2*sizeof(numbers));
  } else {   /* child, reader */
    close(fd[WRITE]);
    read(fd[READ], &numbers, 2*sizeof(numbers));
    printf("Soma: %d\nDiferenca: %d\nMultiplicacao: %d\nDivisao: ", numbers.n1+numbers.n2, numbers.n1-numbers.n2, numbers.n1*numbers.n2);
    if(numbers.n2== 0)
      printf("Divisao por 0 - INVALIDA");
    else
      printf("%f", (double)numbers.n1/numbers.n2);
   }
   exit(0);
}
