#include "utils.h"

static request_t **requests;
static uint32 num_requests;
static time_t init_time;
static int log_fd;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


char *buildLogString( gen_log_t info );
void numToString( char *string , long int number, char decimal_flag);

inline static char isHandled(request_t *request){
	return (request->status & TREATED) || (request->status & DISCARDED);
}


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
  *rejected_fd = open(REJECTED_PATH, O_RDONLY | O_NONBLOCK);
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
			if( !isHandled(requests[i]) ) {
				if( requests[i]->times_rejected < 3 ) {
					if( requests[i]->status & SEND ) {
						pthread_mutex_lock(&mutex);
						requests[i]->status = 0;
						pthread_mutex_unlock(&mutex);
						write(entry_fd, requests[i], sizeof(request_t));
					}
				} else {
					pthread_mutex_lock(&mutex);
					requests[i]->status = DISCARDED;
					pthread_mutex_unlock(&mutex);
				}
				all_handled = 0;
			}
		}
	}
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
			if( !isHandled(requests[i]) )
				all_handled = 0;
		}
		if ( all_handled )
				return 0;
		if(read(*((int*)rejected_fd), &request, sizeof(request_t)) == sizeof(request_t)){
			for (uint32 i = 0 ; i < num_requests ; i++) {
				if(requests[i]->serial_number == request.serial_number){
					pthread_mutex_lock(&mutex);
					memmove(requests[i], &request, sizeof(request_t));
					pthread_mutex_unlock(&mutex);
				}
			}
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

/* ----------------------STRING MANIPULATION SHIT--------------------*/

char *buildLogString( gen_log_t info ){
	char inst[INST_SIZE], pid[PID_SIZE], p[P_SIZE], dur[DUR_SIZE], sep1[]= " - ", sep2[]=": ",
		 *final=(char*)malloc(sizeof(char)*(INST_SIZE+PID_SIZE+P_SIZE+DUR_SIZE+4*SEP1_SIZE+SEP2_SIZE+1));
    memset(inst,' ',INST_SIZE);
    numToString(inst, info.inst,TRUE);
    memset(pid,' ',PID_SIZE);
    numToString(pid, info.pid,FALSE);
    memset(p,' ',P_SIZE);
    numToString(p, info.p,FALSE);
    memset(dur,' ',DUR_SIZE);
    numToString(dur,info.dur,FALSE);

    sprintf(final,"%s%s%s%s%s%s%c%s%s%s%s",inst,sep1,pid,sep1,p,sep2,info.g,sep1,dur,sep1,info.tip);

    return final;
}

void numToString(char *string , long int number, char decimal_flag){
	int i = countNumbers(number)-1, cont = 0;
	if(decimal_flag)
		i++;

	while( i >= 0){
		if (2 == cont && decimal_flag)
			string[i--]='.';
		else{
			string[i--] = (char)(48+(number % 10));
			number = number / 10;
		}
		cont++;
	}
}
