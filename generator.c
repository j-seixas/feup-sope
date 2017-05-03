#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define SEND_NAME "/tmp/entry"
#define ENTRY_NAME "/tmp/rejected"
#define FIFO_MODE 0600

/**
 * @struct send_info_t
 * @brief Struct that will hold a request information
 * @var send_info_t::ser_no
 * Will hold the serial number of this request
 * @var send_info_t::gender
 * The gender of the request (0->F , 1->M)
 * @var send_info_t::time
 * How much time it will spend in the sauna
 */
typedef struct {
	unsigned int ser_no;
	unsigned int gender;
	unsigned long int time;
} send_info_t;

int main (int argc , char *argv[] ){
	if (argc != 4){
		write(STDOUT_FILENO,"Usage: gerador <n. pedidos> <max. utilizacao> <un. tempo>\n",58);
		exit(1);
	}
	srand(time(NULL));
	int n_ped = strtol(argv[1],NULL,10),
	    max_ut= strtol(argv[2],NULL,10),
	    serial_no = 1;
	int write_des , read_des;

	if (mkfifo(SEND_NAME,FIFO_MODE) < 0){
		perror("Error 2");
		exit(2);
	}

	if ( (write_des = open(SEND_NAME , O_WRONLY , NULL)) < 0 ){
		perror("Error 3");
		exit(3);
	}

	if (mkfifo(ENTRY_NAME,FIFO_MODE) < 0){
		perror("Error 4");
		exit(4);
	}
	if ( (read_des = open(ENTRY_NAME , O_RDONLY , NULL)) < 0){
		perror("Error 5");
		exit(5);
	}
	int i=0;
	for (i = 0 ; i < n_ped ; i++){
		send_info_t temp;
		temp.ser_no = serial_no++;
		temp.gender = rand() % 2; //generate numbers between 10 and 1
		temp.time = (rand() % max_ut)+1; //[1,10]
		write(write_des,&temp,sizeof(temp));
	}

}
