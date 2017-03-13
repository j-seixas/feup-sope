#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>  
#include <string.h> 
#include <sys/wait.h>

int main(void) { 

	// WITH PIDs, BUT IT RETURNS BEFORE IT FINISHES -- NOT WORKING
	/*
	pid_t grandparent_pid = getpid();
	if(fork() > 0) { //FATHER
		write(STDOUT_FILENO,"Hello", 5);
	} else { 
		pid_t parent_pid = getpid();
		if(fork() > 0){ //SON (FATHER OF GRANDSON)
			while(grandparent_pid == getppid()){};
			write(STDOUT_FILENO," my ",4);
		} else { //GRANDSON 
			while(parent_pid == getppid()){};
			write(STDOUT_FILENO,"friends!",8);  
		}
	}
	write(STDOUT_FILENO,"\n",1);

	return 0;
	*/
	
	//WITH WAITs
	if(fork() > 0) { //FATHER
		wait(0);
		write(STDOUT_FILENO," friends!\n",10);  
	} else { 
		if(fork() > 0){ //SON (FATHER OF GRANDSON)
			wait(0);
			write(STDOUT_FILENO," my",3);
		} else //GRANDSON 
			write(STDOUT_FILENO,"Hello", 5);
	} 
	return 0; 
	
} 