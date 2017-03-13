#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>  
#include <string.h> 
#include <sys/wait.h>

int main(void) { 
	if(fork() > 0) { 
		wait(0);
		write(STDOUT_FILENO," world!\n",8);  
	} else { 
		write(STDOUT_FILENO,"Hello", 5);
	} 
	return 0; 
} 