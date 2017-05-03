#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main (int argc , char *argv[] ){
	if (argc != 4){
		write(STDOUT_FILENO,"Usage: gerador <n. pedidos> <max. utilizacao> <un. tempo>\n",58);
		exit(1);
	}
	char *fifo_name = "/tmp/entry";
	int n_ped = strtol(argv[1],NULL,10),
	    max_ut= strtol(argv[2],NULL,10);

}
