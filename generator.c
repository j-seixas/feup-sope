#include "utils.h"

static info_t info;
static uint32 num_requests;
static struct timeval init_time;
static int log_fd;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


char *buildLogString( gen_log_t info );
gen_log_t requestToStruct( request_t *req);

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
	info.n_requests[0] = num_requests; info.n_requests[1] = 0; info.n_requests[2] = 0;
	info.n_rejects[0] = 0; info.n_rejects[1] = 0; info.n_rejects[2] = 0;
	info.n_misc[0] = 0; info.n_misc[1] = 0; info.n_misc[2] = 0;
	*max_time  = strtol(argv[2],NULL,10);
	info.requests = malloc(num_requests * sizeof(request_t*));
	for (uint32 i = 0 ; i < num_requests ; i++)
		info.requests[i] = NULL;

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
			if( !isHandled(info.requests[i]) ) {
				if( info.requests[i]->times_rejected < 3 ) {
					if( info.requests[i]->status & SEND ) {
						pthread_mutex_lock(&mutex);
						info.requests[i]->status = 0;
						pthread_mutex_unlock(&mutex);
						write(entry_fd, info.requests[i], sizeof(request_t));

						char *tmp = buildLogString(requestToStruct(info.requests[i]));
						write(log_fd,tmp,sizeof(char)*strlen(tmp));
						printf("SEND - %s",tmp);
					}
				} else {
					pthread_mutex_lock(&mutex);
					info.requests[i]->status = DISCARDED;
					info.n_misc[0]++;
					info.n_misc[ (info.requests[i]->gender == 'M' ? 1 : 2) ]++;
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
			if( !isHandled(info.requests[i]) )
				all_handled = 0;
		}
		if ( all_handled )
			return 0;
		if(read(*((int*)rejected_fd), &request, sizeof(request_t)) == sizeof(request_t)){
			if( request.status & REJECTED){
				info.n_rejects[ (request.gender == 'M' ? 1 : 2) ]++;
				info.n_rejects[0]++;
			}
			char *tmp = buildLogString(requestToStruct(&request));
			write(log_fd,tmp,sizeof(char)*strlen(tmp));
			printf("HAND - %s",tmp);

			pthread_mutex_lock(&mutex);
			memmove(info.requests[request.serial_number], &request, sizeof(request_t));
			pthread_mutex_unlock(&mutex);
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
		info.requests[i] = malloc(sizeof(request_t));
		info.requests[i]->serial_number = i; //serial number will be the same as position in array
		info.requests[i]->gender = rand() % 2 ? 'M' : 'F';
		info.requests[i]->time_spent = (rand() % max_time) + 1;
		info.requests[i]->times_rejected = 0;
		info.requests[i]->status = SEND;
		info.n_requests[ (info.requests[i]->gender == 'M' ? 1 : 2) ]++;
	}
}

/**
 * @brief The main function
 * @param[in] argc How many arguments where passed into the program
 * @param[in] argv The arguments passed into the program
 * @return 0 if all OK, 1 otherwise
 */
int main (int argc , char *argv[] ){
	gettimeofday(&init_time,NULL);
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

	printf("	GENERATED\nTOTAL = %d | M= %d | F= %d\n",info.n_requests[0],info.n_requests[1],info.n_requests[2]);
	printf("	REJECTED\nTOTAL = %d | M= %d | F= %d\n",info.n_rejects[0],info.n_rejects[1],info.n_rejects[2]);
	printf("	DISCARDED\nTOTAL = %d | M= %d | F= %d\n",info.n_misc[0],info.n_misc[1],info.n_misc[2]);

	return 0;
}

/**
 * @brief Builds the string to be printed to the log file
 * @param[in] info Information to be printed to the log file (see struct gen_log_t)
 * @return String to be printed
 */
char *buildLogString( gen_log_t info ){
	char *inst=(char*)malloc(sizeof(char)*INST_SIZE),
		 *pid =(char*)malloc(sizeof(char)*PID_SIZE) ,
		 *p   =(char*)malloc(sizeof(char)*P_SIZE),
		 *dur =(char*)malloc(sizeof(char)*DUR_SIZE),
		 *final=(char*)malloc(sizeof(char)*(INST_SIZE+PID_SIZE+P_SIZE+DUR_SIZE+5*SEP_SIZE+2));

	inst[INST_SIZE]='\0'; 	memset(inst,' ',INST_SIZE);
	numToString(inst, info.inst,TRUE);
	pid[PID_SIZE]='\0'; 	memset(pid,' ',PID_SIZE);
	numToString(pid, info.pid,FALSE);
	p[P_SIZE]='\0'; 		memset(p,' ',P_SIZE);
	numToString(p, info.p,FALSE);
	dur[DUR_SIZE]='\0';		memset(dur,' ',DUR_SIZE);
	numToString(dur,info.dur,FALSE);

	sprintf(final,"%s | %s | %s : %c | %s | %s\n",inst,pid,p,info.g,dur,info.tip);
	free(inst);
	free(pid);
	free(p);
	free(dur);
	return final;
}

gen_log_t requestToStruct( request_t *req){
	gen_log_t tmp;
	tmp.inst = microDifference(init_time);
	tmp.pid = getpid();
	tmp.p = req->serial_number;
	tmp.g = req->gender;
	tmp.dur = req->time_spent;
	tmp.tip = ((req->status & SEND || req->status & TREATED) ? "PEDIDO" :
		((req->status & REJECTED) ? "REJEITADO" : "DESCARTADO"));

	return tmp;
}
