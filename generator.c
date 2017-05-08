#include "utils.h"

static Request **requests;
static uint32 num_requests;
static uint64 max_time;

int init(int argc , char *argv[]) {
	if (argc != 3){
		printf("Usage: generator <num. requests> <max. time>\n");
		return 1;
	}
	srand(time(NULL));
	num_requests = strtol(argv[1],NULL,10);
	max_time  = strtol(argv[2],NULL,10);
	requests = malloc(num_requests * sizeof(Request*));
	for (uint32 i = 0 ; i < num_requests ; i++)
		requests[i] = NULL;
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

char requestsHandled() {
	for (uint32 i = 0 ; i < num_requests ; i++){
		if(requests[i] != NULL)
			return 0;
	}
	return 1;
}

void sendRequests(int entry_fd){
	while( !requestsHandled() ){
		for (uint32 i = 0 ; i < num_requests ; i++){
			if(requests[i] != NULL) {
				if(requests[i]->times_rejected < 3
				&& requests[i]->resend_flag) {
					requests[i]->resend_flag = 0;
					printf("Serial: %lu, Rejected: %d, Flag: %d\n", requests[i]->serial_number, requests[i]->times_rejected, requests[i]->resend_flag);
					write(entry_fd, requests[i], sizeof(Request));
				}
				else
					requests[i] = NULL;
			}
		}
	}
}

void* handleResults(void* rejected_fd){
	Request request;
	while( !requestsHandled() ){
		read(*((int*)rejected_fd), &request, sizeof(Request));
		for (uint32 i = 0 ; i < num_requests ; i++){
			if(requests[i]->serial_number == request.serial_number) {
				memmove(requests[i], &request, sizeof(Request));
				break;
			}
		}
	}
	return 0;
}

pthread_t initResultReader(int rejected_fd) {
	pthread_t thread;
	pthread_create(&thread, NULL, handleResults, &rejected_fd);
	return thread;
}

void generateRequests() {
	for (uint32 i = 0 ; i < num_requests ; i++){
		requests[i] = malloc(sizeof(Request));
		requests[i]->serial_number = i;
		requests[i]->gender = rand() % 2 ? 'M' : 'F';
		requests[i]->time_spent = (rand() % max_time) + 1;
		requests[i]->times_rejected = 0;
		requests[i]->resend_flag = 1;
		printf("Serial: %lu, Gender: %c, Time: %lu\n", requests[i]->serial_number, requests[i]->gender, requests[i]->time_spent);
	}
}

int main (int argc , char *argv[] ){
	int rejected_fd;
  int entry_fd;

	if( init(argc, argv) )
		exit(1);

	createFifos();

	if( openFifos(&rejected_fd, &entry_fd) )
		exit(1);
	printf("Opened fifos\n");

	generateRequests();
	pthread_t thread = initResultReader(rejected_fd);
	sendRequests(entry_fd);

	pthread_join(thread, NULL);

	if( closeFifos(rejected_fd, entry_fd) )
		exit(1);
	printf("Closed fifos\n");

	return 0;
}
