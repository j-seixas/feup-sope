#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
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

/**
 * @struct queue_t
 * @brief Struct to simulate the behaviour of a queue_t
 * @var queue_t::ptr
 *  	Element to pop
 * @var queue_t::insert
 * 	Where to insert next element
 * @var queue_t::delta_ops
 * 	Difference between push and pops
 * @var queue_t::arr
 * 	Array which will hold the information
 * @detail 
 * 	If queue_t::delta_ops=3 then it is impossible to push a new element until an element is popped
 */
typedef struct{
	unsigned int ptr;
	unsigned int insert;
	unsigned int delta_ops;
	send_info_t arr[3];
} queue_t;


int sendRequest( int write_fd , unsigned int serial_no , unsigned long int max_ut);
int checkResponse( int read_fd );

/**
 * @brief Pushes a new element in the queue
 * @param q Pointer to the queue
 * @param elem Element to be inserted into queue
 * @return 0 if push was successful, 1 if push is not possible
 */
int queuePush (queue_t *q, send_info_t *elem){
	if (q->delta_ops < 3){
		q->arr[q->insert++] = *elem;
		q->insert = q->insert % 3;
		q->delta_ops++;
		return 0;
	}
	else
		return 1;
}


send_info_t queuePop(queue_t *q){
	send_info_t ret = q->arr[q->ptr];
	if (ret != NULL && q->delta_ops > 0){
		q->arr[q->ptr++] = NULL;	
		q->ptr = q->ptr % 3;
		q->delta_ops--;
	}
	return ret;
}

int main (int argc , char *argv[] ){
	if (argc != 4){
		printf("Usage: gerador <n. pedidos> <max. utilizacao> <un. tempo>\n");
		exit(1);
	}
	srand(time(NULL));

	int n_ped = strtol(argv[1],NULL,10),
	    write_fd,
	    read_fd;
	unsigned long int max_ut= strtol(argv[2],NULL,10),
			  serial_no = 1;

	if (mkfifo(SEND_NAME,FIFO_MODE) < 0){
		perror("Error 2");
		exit(2);
	}

	if ( (write_fd = open(SEND_NAME , O_WRONLY , NULL)) < 0 ){
		perror("Error 3");
		exit(3);
	}

	if (mkfifo(ENTRY_NAME,FIFO_MODE) < 0){
		perror("Error 4");
		exit(4);
	}
	if ( (read_fd = open(ENTRY_NAME , O_RDONLY , NULL)) < 0){
		perror("Error 5");
		exit(5);
	}
	int i=0;
	for (i = 0 ; i < n_ped ; i++){
		checkResponse( read_fd );
		sendRequest( write_fd , serial_no++ , max_ut);
		send_info_t temp;
		temp.ser_no = serial_no++;
		temp.gender = rand() % 2; //generate numbers between 10 and 1
		temp.time = (rand() % max_ut)+1; //[1,10]
		write(write_fd,&temp,sizeof(temp));
	}


	return 0;

}

/**
 * @brief Sends a single request to the sauna
 * @param write_fd File descriptor to write information to
 * @param serial_no Serial number of the request
 * @param max_ut Limits the generated time between [1,max_ut]
 */
int sendRequest( int write_fd , unsigned int serial_no , unsigned long int max_ut){
	send_info_t temp;
	temp.ser_no = serial_no;
	temp.gender = rand() % 2; //generate numbers between 0 and 1
	temp.time = (rand() % max_ut)+1; //[1,10]
	write(write_fd,&temp,sizeof(temp));


	return 0;
}

/**
 * @brief Checks if there is a response from the rejected FIFO
 * @param read_fd File descriptor of the FIFO
 * @return What the response should be
 */
int checkResponse( int read_fd ){
	unsigned int n_bytes=0;
	send_info_t response;
	response.ser_no=0;
	while ( (n_bytes = read(read_fd,&response,sizeof(response))) > 0){};
	if (0 != response.ser_no){ //it got a message from rejected
		//TODO Do some shit
		write(STDOUT_FILENO,"MERDA\n",6);
	}

	return 0;
}


