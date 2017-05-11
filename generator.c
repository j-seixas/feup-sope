#include "utils.h"
#define IS_HANDLED(n) (((n) & TREATED) || ((n) & DISCARDED))


static request_t **requests;
static uint32 num_requests;
static time_t init_time;
static int log_fd;

/**
 * @brief Initializes needed variables and checks number of arguments
 * @param[in] argc How many arguments were passed into the program
 * @param[in] argv The arguments passed into the program
 * @param[out] max_time Maximum amount of time that a user can spend in the sauna
 * @return 0 if all OK , 1 if number of arguments is not correct
 */
int init(int argc , char *argv[], uint64 *max_time) {
	if (argc != 3){
		printf("Usage: generator <num. requests> <max. time>\n");
		return 1;
	}
	srand(time(NULL));
	num_requests = strtol(argv[1],NULL,10);
	*max_time  = strtol(argv[2],NULL,10);
	requests = malloc(num_requests * sizeof(request_t*));
	for (uint32 i = 0 ; i < num_requests ; i++)
		requests[i] = NULL;

	if ((log_fd = openLogFile( GEN_LOGFILE )) == -1){
		perror("Error opening generator log file ");
		exit(2);
	}
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

/**
 * @brief Sends all requests to the sauna
 * @param[in] entry_fd File descriptor to send the requests to
 * @detail This function only exits when all requests where handled which means they were either accepted or discarded.
 * 	   It continuously iterates the whole array searching for an unhandled request.
 */
void sendRequests(int entry_fd){
	char all_handled = 0;
	while( !all_handled ){
		all_handled = 1;
		for (uint32 i = 0 ; i < num_requests ; i++){
			if( !IS_HANDLED(requests[i]->status) ) {
				if( requests[i]->times_rejected < 3 ) {
					if( requests[i]->status & SEND ) {
						requests[i]->status = 0;
						printf("Sender -> Serial: %lu, Rejected: %d, Status: %d\n", requests[i]->serial_number, requests[i]->times_rejected, requests[i]->status);
						write(entry_fd, requests[i], sizeof(request_t));
					}
				} else {
					requests[i]->status = DISCARDED;
				}
				all_handled = 0;
			}
		}
	}
	printf("Sender ended\n");
}

/**
 * @brief Processes the requests returned by the sauna
 * @param[in] rejected_fd Pointer to file descriptor to read the requests from
 * @return Always 0 when all requests were handled
 */
void* handleResults(void* rejected_fd){
	request_t request;
	while( 1 ) {
		char all_handled = 1;
		for (uint32 i = 0 ; i < num_requests ; i++) {
			if( !IS_HANDLED(requests[i]->status) )
				all_handled = 0;
		}
		if ( all_handled ) {
			printf("Handler ended\n");
			return 0;
		}
		read(*((int*)rejected_fd), &request, sizeof(request_t));
		printf("Handler -> Serial: %lu, Rejected: %d, Status: %d\n", request.serial_number, request.times_rejected, request.status);
		for (uint32 i = 0 ; i < num_requests ; i++) {
			if(requests[i]->serial_number == request.serial_number)
				memmove(requests[i], &request, sizeof(request_t));
		}
	}
}

/**
 * @brief Generates a given number of requests
 * @param[in] max_time Maximum amount of time a single user is allowed to spend inside the sauna
 * @detail The number of requests generated is given by the static variable "num_requests", the gender and time spent are generated randomly
 */
void generateRequests(uint64 max_time) {
	for (uint32 i = 0 ; i < num_requests ; i++){
		requests[i] = malloc(sizeof(request_t));
		requests[i]->serial_number = i;
		requests[i]->gender = rand() % 2 ? 'M' : 'F';
		requests[i]->time_spent = (rand() % max_time) + 1;
		requests[i]->times_rejected = 0;
		requests[i]->status = SEND;
		printf("Generator -> Serial: %lu, Gender: %c, Time: %lu\n", requests[i]->serial_number, requests[i]->gender, requests[i]->time_spent);
	}
}

/**
 * @brief The main function
 * @param[in] argc How many arguments where passed into the program
 * @param[in] argv The arguments passed into the program
 * @return 0 if all OK, 1 otherwise
 */
int main (int argc , char *argv[] ){
	init_time=time(NULL);
	int rejected_fd;
  	int entry_fd;
  	uint64 max_time;

	if( init(argc, argv, &max_time) )
		exit(1);

	createFifos();

	if( openFifos(&rejected_fd, &entry_fd) )
		exit(1);
	printf("Opened fifos\n");

	generateRequests(max_time);
	pthread_t thread;
	pthread_create(&thread, NULL, handleResults, &rejected_fd);
	sendRequests(entry_fd);

	pthread_join(thread, NULL);

	if( closeFifos(rejected_fd, entry_fd) )
		exit(1);
	printf("Closed fifos\n");

	return 0;
}
