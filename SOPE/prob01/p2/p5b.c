#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[], char* envp[]){
	int i = 0;
	while(envp[i] != NULL){
		if(strncmp(envp[i], "USER=", 5) == 0){
			int j= 5;
			printf("Hello ");
			for(; j < strlen(envp) ; j++)
				printf("%s", envp[i][j]);
			printf("!\n");
		}
	}
	return 0;
}