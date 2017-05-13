#include "utils.h"

static info_t info;
static uint32 num_seats;
static uint32 num_seats_available;
static char curr_gender;
static int log_fd;
static struct timeval init_time;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char *buildLogString( sauna_log_t info );
sauna_log_t requestToStruct( request_t *req);

inline static char hasSeats(){
  return num_seats_available > 0;
}

inline static char sameGender(request_t *request) {
  return curr_gender == 0 || curr_gender == request->gender;
}

/**
 *  @brief       Reads a request from the entry fifo
 *  @param[out]  request   The pointer to which a request will be read to
 *  @param[in]   entry_fd  The file descriptor of the entry fifo
 *  @return      Returns whether or not the reading was successful
 */
int readRequest( request_t *request, int entry_fd ) {
  return read(entry_fd, request, sizeof(request_t)) == sizeof(request_t);
}

/**
 *  @brief      Rejects a user
 *  @param[in]  request      The rejected request
 *  @param[in]  rejected_fd  The file descriptor of the rejected fifo
 *  @return     Returns whether or not the writing was successful
 */
int sendResult( request_t *request, int rejected_fd ) {
  return write(rejected_fd, request, sizeof(request_t)) == sizeof(request_t);
}

/**
 *  @brief       Processes the arguments from the command line and initializes variables
 *  @param[in]   argc  The number of command line arguments
 *  @param[in]   argv  The command line arguments
 *  @return      Returns whether or not the arguments are valid
 */
int init(const int argc, char *argv[]) {
  if ( argc != 2 ) {
    printf("Usage: sauna <num. seats>\n");
    return 1;
  }

  num_seats = strtol(argv[1],NULL,10);
  num_seats_available = num_seats;
  info.requests = malloc(num_seats * sizeof(request_t*));
	info.n_requests[0] = 0; info.n_requests[1] = 0; info.n_requests[2] = 0;
	info.n_rejects[0] = 0; info.n_rejects[1] = 0; info.n_rejects[2] = 0;
	info.n_misc[0] = 0; info.n_misc[1] = 0; info.n_misc[2] = 0;
  curr_gender = 0;
  for(uint32 i = 0; i < num_seats; i++)
    info.requests[i] = NULL;

  if ((log_fd = openLogFile( SAUNA_LOGFILE )) == -1){
    perror("Error opening sauna log file ");
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
  *rejected_fd = open(REJECTED_PATH, O_WRONLY);
  *entry_fd = open(ENTRY_PATH, O_RDONLY);
  return (*rejected_fd < 0 || *entry_fd < 0);
}

void* waitAndLeave( void *pos ){
  usleep( info.requests[*(uint32 *)pos]->time_spent);
  pthread_mutex_lock(&mutex);
  	num_seats_available++;
  	if(num_seats_available == num_seats)
    	curr_gender = 0;
  	info.requests[*(uint32 *)pos] = NULL;
  pthread_mutex_unlock(&mutex);
  return (void*)0;
}

/**
 *  @brief       Lets a user enter the sauna
 *  @param[in]  request  The accepted user request
 */
void enter( request_t *request ) {
  pthread_mutex_lock(&mutex);
  num_seats_available--;
  curr_gender = request->gender;
  pthread_mutex_unlock(&mutex);
	uint32 *i = (uint32 *)malloc(sizeof(uint32));
  for(*i = 0; *i < num_seats; (*i)++){
    if(info.requests[*i] == NULL){
      info.requests[*i] = malloc(sizeof(request_t));
      request->status = TREATED;
      memmove(info.requests[*i], request, sizeof(request_t));
      break;
    }
  }
  pthread_t thread;
  pthread_create(&thread, NULL, waitAndLeave, i);
}

/**
 * @brief Puts a request waiting for a free seat
 */
void putOnHold() {
  while(1) {
    for(uint32 i = 0; i < num_seats; i++)
      if(info.requests[i] == NULL)
        return;
    }
}

void reject(request_t *request){
  request->status = (REJECTED | SEND);
  request->times_rejected++;
}


int main(int argc, char *argv[]) {
  gettimeofday(&init_time, NULL);
  int rejected_fd;
  int entry_fd;
  request_t *request = malloc(sizeof(request_t));

  if ( init(argc, argv) )
    exit(1);
  createFifos();

  if ( openFifos(&rejected_fd, &entry_fd) )
    exit(1);

  while( readRequest(request, entry_fd) ) {
		info.n_requests[0]++;
		info.n_requests[ (request->gender == 'M' ? 1 : 2) ]++;

    char *tmp = buildLogString(requestToStruct(request));
    write(log_fd,tmp,sizeof(char)*strlen(tmp));
    printf("%s",tmp);

    if( sameGender(request) ) {
      if ( hasSeats() )
        enter(request);
      else {
        putOnHold();
        enter(request);
      }
			info.n_misc[0]++;
			info.n_misc[ (request->gender == 'M' ? 1 : 2) ]++;
    } else{
      reject(request);
			info.n_rejects[0]++;
			info.n_rejects[ (request->gender == 'M' ? 1 : 2) ]++;
		}

    tmp = buildLogString(requestToStruct(request));
    write(log_fd,tmp,sizeof(char)*strlen(tmp));
    printf("%s",tmp);

    sendResult(request, rejected_fd);
  }
  if ( closeFifos(rejected_fd, entry_fd) )
    exit(1);

	printf("	RECEIVED\nTOTAL = %d | M= %d | F= %d\n",info.n_requests[0],info.n_requests[1],info.n_requests[2]);
	printf("	REJECTED\nTOTAL = %d | M= %d | F= %d\n",info.n_rejects[0],info.n_rejects[1],info.n_rejects[2]);
	printf("	SERVED\nTOTAL = %d | M= %d | F= %d\n",info.n_misc[0],info.n_misc[1],info.n_misc[2]);
  pthread_exit(NULL);
  return 0;
}


char *buildLogString( sauna_log_t info ){
	int final_size = INST_SIZE+PID_SIZE+TID_SIZE+P_SIZE+DUR_SIZE+6*SEP_SIZE+2;
  char *inst =(char*)malloc(sizeof(char)*INST_SIZE),
       *pid  =(char*)malloc(sizeof(char)*PID_SIZE),
       *tid  =(char*)malloc(sizeof(char)*TID_SIZE),
       *p    =(char*)malloc(sizeof(char)*P_SIZE),
       *dur  =(char*)malloc(sizeof(char)*DUR_SIZE),
       *final=(char*)malloc(sizeof(char)*final_size);

  inst[INST_SIZE]='\0';   memset(inst,' ',INST_SIZE);
  numToString(inst, info.inst,TRUE);
  pid[PID_SIZE]='\0';     memset(pid,' ',PID_SIZE);
  numToString(pid, info.pid,FALSE);
  tid[TID_SIZE]='\0';     memset(tid,' ',TID_SIZE);
  numToString(tid,info.tid,FALSE);
  p[P_SIZE]='\0';         memset(p,' ',P_SIZE);
  numToString(p, info.p,FALSE);
  dur[DUR_SIZE]='\0';     memset(dur,' ',DUR_SIZE);
  numToString(dur,info.dur,FALSE);

  sprintf(final,"%s | %s | %s | %s : %c | %s | %s\n",inst,pid,tid,p,info.g,dur,info.tip);
  free(inst);
  free(pid);
  free(tid);
  free(p);
  free(dur);
  return final;
}

sauna_log_t requestToStruct(request_t *req){
  sauna_log_t tmp;
  tmp.inst = microDifference(init_time);
  tmp.pid = getpid();
  tmp.tid = (uint64)pthread_self();
  tmp.p = req->serial_number;
  tmp.g = req->gender;
  tmp.dur = req->time_spent;
  tmp.tip = (req->status & TREATED)  ? "TREATED"   :
            (req->status & REJECTED) ? "REJECTED" : "RECEIVED";

  return tmp;
}
