// PROGRAMA p7.c 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]){ 
   char prog[20]; 
   sprintf(prog,"%s.c",argv[1]); 
   execlp("gc","gcc",prog,"-Wall","-o",argv[1],NULL); 
   printf("Isto nao vai ser executado\n"); 
   exit(0); 
} 