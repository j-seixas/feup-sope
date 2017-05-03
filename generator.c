#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define FIFO_NAME "/tmp/entry"
#define FIFO_MODE 0600

int main (int argc , char *argv[] ){
	if (argc != 4){
		printf("Usage: gerador <n. pedidos> <max. utilizacao> <un. tempo>\n");
		exit(1);
	}
	char *fifo_name = "/tmp/entry";
	int n_ped = strtol(argv[1],NULL,10),
	    max_ut= strtol(argv[2],NULL,10),
	    serial_n = 1;

	if (mkfifo(fifo_name,FIFO_MODE) < 0){
		perror("Error 2");
		exit(2);
	}
	if ()



}
