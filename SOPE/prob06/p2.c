// PROGRAMA p1b.c NOT WORKING
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define STDERR 2
#define NUMITER 10000
int num = 50000;
void * thrfunc(void * arg)
{
  int* i = 0;
  fprintf(stderr, "Starting thread %s\n", (char *) arg);
  while(num != 0){

    write(STDERR,arg,1);
    num--;
    i++;
  }
  return (void *)i;
}
int main()
{
  pthread_t ta, tb;
  char* var1 = (char*) malloc(sizeof(char));
  (*var1) = '1';
  char* var2 = (char*) malloc(sizeof(char));
  (*var2) = '2';
  pthread_create(&ta, NULL, thrfunc, var1);
  pthread_create(&tb, NULL, thrfunc, var2);
  int * res1 = 0, *res2 = 0;
  pthread_join(ta, (void *)res1);
  pthread_join(tb, (void *)res2);
  printf("1 - %d | 2 - %d | S = %d \n", (*res1), (*res2), (*res1)+(*res2));
  return 0;
}
