#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>  
#include <string.h> 

int main(void) { 
	pid_t parent_pid = getpid();
	if(fork() > 0) { 
		write(STDOUT_FILENO,"Hello", 5);		 
	} else { 
		while(parent_pid == getppid()){};
		write(STDOUT_FILENO," world!\n",8); 
	} 
	return 0; 
} 