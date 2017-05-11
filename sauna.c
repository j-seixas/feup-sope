#include "utils.h"

static request_t **requests;
static uint32 num_seats;
static uint32 num_seats_available;
static char curr_gender;
static int log_fd;

#define HAS_SEATS (num_seats_available>0)
#define SAME_GENDER(g) ( (curr_gender==0) || (curr_gender == (g)))

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
  requests = malloc(num_seats * sizeof(request_t*));
  curr_gender = 0;
  for(uint32 i = 0; i < num_seats; i++)
    requests[i] = NULL;

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

void* waitAndLeave( void *serial_number ){
  for(uint32 i = 0; i < num_seats; i++) {
    if(requests[i] != NULL) {
      if(requests[i]->serial_number == *((uint64*)serial_number)){
        uint64 sleep_time = requests[i]->time_spent;
        printf("Going to sleep %luus\n", sleep_time);
        usleep(sleep_time);
        printf("Finished sleeping\n");
        num_seats_available++;
        if(num_seats_available == num_seats)
          curr_gender = 0;
        requests[i] = NULL;
        free(serial_number);
        return (void*)0;
      }
    }
  }
  return (void*)1;
}

/**
 *  @brief       Lets a user enter the sauna
 *  @param[in]  request  The accepted user request
 */
void enter( request_t *request ) {
  num_seats_available--;
  curr_gender = request->gender;
  uint64 *serial_number = malloc(sizeof(uint64));
  for(uint32 i = 0; i < num_seats; i++)
    if(requests[i] == NULL){
      requests[i] = malloc(sizeof(request_t));
      request->status = TREATED;
      memmove(requests[i], request, sizeof(request_t));
      *serial_number = requests[i]->serial_number;
      break;
  }
  pthread_t thread;
  pthread_create(&thread, NULL, waitAndLeave, (void*)serial_number);
}

/**
 * @brief Puts a request waiting for a free seat
 */
void putOnHold() {
  while(1) {
    for(uint32 i = 0; i < num_seats; i++)
      if(requests[i] == NULL)
        return;
  }
}

void reject(request_t *request){
  request->status = (REJECTED | SEND);
  request->times_rejected++;
}


int main(int argc, char *argv[]) {
  int rejected_fd;
  int entry_fd;
  request_t *request = malloc(sizeof(request_t));

  if ( init(argc, argv) )
    exit(1);
  createFifos();
  if ( openFifos(&rejected_fd, &entry_fd) )
    exit(1);
  printf("Opened fifos\n");


  while( readRequest(request, entry_fd) ) {
    printf("\nSerial: %lu, Gender: %c, Time: %lu, Rejected: %d\n", request->serial_number, request->gender, request->time_spent, request->times_rejected);

  if( SAME_GENDER(request->gender) ) {
    printf("Same gender\n");
    if ( HAS_SEATS ){
      printf("Has seats\n");
      enter(request);
      printf("Entered\n");
    } else {
      printf("Waiting for free spot\n");
      putOnHold();
      printf("Spot freed\n");
      enter(request);
      printf("Entered\n");
    }
  } else {
      reject(request);
      printf("Rejected\n");
  }

    sendResult(request, rejected_fd);
    printf("Result sent\n");
  }

  if ( closeFifos(rejected_fd, entry_fd) )
    exit(1);
  printf("Closed fifos\n");

  pthread_exit(NULL);

  return 0;
}
