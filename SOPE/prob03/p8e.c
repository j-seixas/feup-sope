// PROGRAMA p8.c 
#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h> 

int checkStatus(int *status){
  if(WIFEXITED(status) || WIFSIGNALED(status))
    return WEXITSTATUS;
}


int main(int argc, char *argv[], char *envp[]) 
{ 
  int status;
   pid_t pid; 
   if (argc != 2) { 
     printf("usage: %s dirname\n",argv[0]); 
     exit(1); 
   } 
   pid=fork(); 
   if (pid > 0){ 
     printf("My child is going to execute command  \"ls -laR %s\"\n", argv[1]); 
     wait(&status);
   }else if (pid == 0){
     char* const a[4] = {"/bin/ls", "-laR", argv[1], NULL} ;
     execve("/bin/ls", a, envp); 
     printf("Command not executed !\n"); 
     exit(1); 
   } 
   exit(status); 
} 