#include "utils.h"

static Request **requests;
static uint32 num_requests;
static uint64 max_time;
static uint64 serial_no;

int init(int argc , char *argv[]) {
	if (argc != 3){
		printf("Usage: generator <num. requests> <max. time>\n");
		return 1;
	}
	srand(time(NULL));
	num_requests = strtol(argv[1],NULL,10);
	max_time  = strtol(argv[2],NULL,10);
	serial_no = 1;
	requests = malloc(num_requests * sizeof(Request*));
	return 0;
}

/**
 *  @brief       Opens the entry and rejected fifos
 *  @param[out]  rejected_fd  The pointer to which the file descriptor of the rejected fifo will be written to
 *  @param[out]  entry_fd     The pointer to which the file descriptor of the entry fifo will be written to
 *  @return      Returns whether or not the fifos were opened
 */
int openFifos(int *rejected_fd, int *entry_fd) {
  *rejected_fd = open(REJECTED_PATH, O_RDONLY);
  *entry_fd = open(ENTRY_PATH, O_WRONLY);
  return (*rejected_fd < 0 || *entry_fd < 0);
}

void sendRequests(int entry_fd){
	for (uint32 i = 0 ; i < num_requests ; i++){
		write(entry_fd, requests[i], sizeof(Request));
	}
}

void sendRejected(int entry_fd){
	char allSent;
	do{
		allSent = 1;
		for (uint32 i = 0 ; i < num_requests ; i++){
			if(requests[i] != NULL){
				allSent = 0;
				if(requests[i]->times_rejected == 3)
					requests[i] = NULL;
				else
					write(entry_fd, requests[i], sizeof(Request));
			}
		}
	} while(!allSent);
}

void generateRequests() {
	for (uint32 i = 0 ; i <= num_requests ; i++){
		requests[i] = malloc(sizeof(Request));
		requests[i]->serial_number = serial_no++;
		requests[i]->gender = rand() % 2 ? 'M' : 'F';
		requests[i]->time_spent = (rand() % max_time) + 1;
		requests[i]->times_rejected = 0;
		printf("Generating Request: %d\n. Serial: %lu, ", i, requests[i]->serial_number);
		printf("Gender: %c, Time: %lu, Rejected: %d", requests[i]->gender, requests[i]->time_spent, requests[i]->times_rejected);
	}
}

void* readRejected(void* rejected_fd){
	Request request;
	while( read(*((int*)rejected_fd), &request, sizeof(Request)) ) {
		for (uint32 i = 0 ; i < num_requests ; i++){
			if(requests[i]->serial_number == request.serial_number){
				requests[i]->times_rejected++;
				break;
			}
		}
	}
	return 0;
}

void initReader(int rejected_fd) {
	pthread_t thread;
	pthread_create(&thread, NULL, readRejected, &rejected_fd);
}

int main (int argc , char *argv[] ){
	int rejected_fd;
  int entry_fd;

	if( init(argc, argv) )
		exit(1);

	createFifos();
	printf("Generator created fifos\n");

	if( openFifos(&rejected_fd, &entry_fd) )
		exit(1);
	printf("Generator opened fifos\n");

	generateRequests();
	initReader(rejected_fd);
	sendRequests(entry_fd);
	sendRejected(entry_fd);

	if( closeFifos(rejected_fd, entry_fd) )
		exit(1);

	pthread_exit(NULL);

	return 0;

}
