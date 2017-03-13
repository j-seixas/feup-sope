#include <stdio.h>


int main(int argc, char* argv[]){
	int i = atoi(argv[1]);
	int j = 2;
	for(; i > 0 ; i--){
		printf("Hello");
		for(; j < argc ; j++){
			printf(" %s", argv[j]);
		}
		j = 2;
		printf("!\n");
	}
	return 0;
}