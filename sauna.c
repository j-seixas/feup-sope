#include "utils.h"

static Request **requests;
static uint32 num_seats;
static uint32 num_seats_available;
static char curr_gender;
static pid_t main_thread_tid;

/**
 *  @brief       Reads a request from the entry fifo
 *  @param[out]  request   The pointer to which a request will be read to
 *  @param[in]   entry_fd  The file descriptor of the entry fifo
 *  @return      Returns whether or not the reading was successful
 */
int readRequest( Request *request, int entry_fd ) {
  return read(entry_fd, request, sizeof(Request)) == sizeof(Request);
}

void* waitForUser( void *request ){
  usleep(((Request*)request)->time_spent);
  for(uint32 i = 0; i < num_seats; i++)
    if(requests[i] != NULL)
      if(requests[i]->serial_number == ((Request*)request)->serial_number)
        requests[i]->serial_number = 0;
  pthread_kill(main_thread_tid, SIGUSR1);
  return 0;
}

/**
 *  @brief       Lets a user enter the sauna
 *  @param[in]   request              The accepted user request
 *  @param[out]  num_seats_available  The number of seats available in sauna
 *  @param[out]  curr_gender          The current gender of the people inside the sauna
 */
void enter( Request *request ) {
  num_seats_available--;
  curr_gender = request->gender;
  for(uint32 i = 0; i < num_seats; i++)
    if(requests[i] == NULL)
      memcpy(requests[i], request, sizeof(Request));
  pthread_t thread;
  pthread_create(&thread, NULL, waitForUser, (void*)request);
}

/**
 *  @brief      Rejects a user
 *  @param[in]  request      The rejected request
 *  @param[in]  rejected_fd  The file descriptor of the rejected fifo
 *  @return     Returns whether or not the writing was successful
 */
int reject( Request *request, int rejected_fd ) {
    return write(rejected_fd, request, sizeof(Request)) == sizeof(Request);
}

/**
 *  @brief       Processes the arguments from the command line
 *  @param[out]  num_seats        The pointer to which the number of seats of the sauna will be written to
 *  @param[in]   argc             The number of command line arguments
 *  @param[in]   argv             The command line arguments
 *  @return      Returns whether or not the arguments are valid
 */
int readArgs(const int argc, char *argv[]) {
  if ( argc != 2 ) {
    printf("Usage: sauna <num. seats>\n");
    return 1;
  }

  num_seats = strtol(argv[1],NULL,10);
  num_seats_available = num_seats;
  requests = malloc(num_seats * sizeof(Request*));
  curr_gender = 0;
  main_thread_tid = syscall(SYS_gettid);

  for(uint32 i = 0; i < num_seats; i++)
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
  *rejected_fd = open(REJECTED_PATH, O_WRONLY);
  *entry_fd = open(ENTRY_PATH, O_RDONLY);
  return (*rejected_fd < 0 || *entry_fd < 0);
}

/**
 *  @brief       Processes the leave of a user
 *  @param[in]   request              The accepted user request
 *  @param[out]  num_seats_available  The number of seats available in sauna
 *  @param[out]  curr_gender          The current gender of the people inside the sauna
 *  @param[in]   num_seats            The number of seats of the sauna
 */
void leave() {
  num_seats_available++;
  if(num_seats_available == num_seats)
    curr_gender = 0;
  for(uint32 i = 0; i < num_seats; i++)
    if(requests[i] != NULL)
      if(requests[i]->time_spent == 0){
        //TODO log
        requests[i] = NULL;
      }
}

void putOnHold() {
  pause();
}

/**
 *  @brief  Handles the signal sent when a user leaves the sauna
 *  @param  signal  The signal received
 */
void timeupHandler(int signal) {
    leave();
}

int hasSeats() {
  return num_seats_available > 0;
}

int sameGender(Request *request) {
  return (curr_gender == 0)
      || (curr_gender == request->gender);
}

/**
 *  @brief   Installs the signal handler for when a user leaves the sauna
 *  @return  Returns whether or not the signal handler was installed
 */
int installTimeupHandler() {
  struct sigaction new_action;
  new_action.sa_handler = timeupHandler;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags = 0;
  return sigaction(SIGUSR1, &new_action, NULL);
}

int main(int argc, char *argv[]) {
  int rejected_fd;
  int entry_fd;
  Request request;

  if ( installTimeupHandler() )
    exit(1);
  if ( readArgs(argc, argv) )
    exit(1);
  createFifos();
  if ( openFifos(&rejected_fd, &entry_fd) )
    exit(1);
  while( readRequest(&request, entry_fd) ) {
    if( sameGender(&request) ) {
      if ( hasSeats() )
        enter(&request);
      else {
        putOnHold();
        enter(&request);
      }
    } else
      reject(&request, rejected_fd);
  }
  if ( closeFifos(rejected_fd, entry_fd) )
    exit(1);

  pthread_exit(NULL);

  return 0;
}
