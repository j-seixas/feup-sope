#include "utils.h"

static Request **requests;
static uint32 num_seats;
static uint32 num_seats_available;
static char curr_gender;
//static pid_t main_thread_tid;

/**
 *  @brief       Reads a request from the entry fifo
 *  @param[out]  request   The pointer to which a request will be read to
 *  @param[in]   entry_fd  The file descriptor of the entry fifo
 *  @return      Returns whether or not the reading was successful
 */
int readRequest( Request *request, int entry_fd ) {
  return read(entry_fd, request, sizeof(Request)) == sizeof(Request);
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
 *  @brief       Processes the arguments from the command line and initializes variables
 *  @param[in]   argc             The number of command line arguments
 *  @param[in]   argv             The command line arguments
 *  @return      Returns whether or not the arguments are valid
 */
int init(const int argc, char *argv[]) {
  if ( argc != 2 ) {
    printf("Usage: sauna <num. seats>\n");
    return 1;
  }

  num_seats = strtol(argv[1],NULL,10);
  num_seats_available = num_seats;
  requests = malloc(num_seats * sizeof(Request*));
  curr_gender = 0;
  //main_thread_tid = syscall(SYS_gettid);

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
 *  @param[in]   request  The accepted user request
 */
void leave(Request *request) {
  num_seats_available++;
  if(num_seats_available == num_seats)
    curr_gender = 0;
  for(uint32 i = 0; i < num_seats; i++)
    if(requests[i] != NULL)
      if(requests[i]->serial_number == request->serial_number) {
        //TODO log
        requests[i] = NULL;
      }
}

void* waitForUser( void *request ){
  usleep(((Request*)request)->time_spent);
  leave(request);
  return 0;
}

/**
 *  @brief       Lets a user enter the sauna
 *  @param[in]  request  The accepted user request
 */
void enter( Request *request ) {
  num_seats_available--;
  curr_gender = request->gender;
  for(uint32 i = 0; i < num_seats; i++)
    if(requests[i] == NULL){
      printf("Found empty spot at %d\n", i);
      requests[i] = malloc(sizeof(Request));
      memmove(requests[i], request, sizeof(Request));
      printf("Copied request to array\n");
      break;
    }
  waitForUser((void*)request);
  //pthread_t thread;
  //pthread_create(&thread, NULL, waitForUser, (void*)request);
}

void putOnHold() {
  while(1) {
    for(uint32 i = 0; i < num_seats; i++)
      if(requests[i] == NULL)
        return;
  }
}

int hasSeats() {
  return num_seats_available > 0;
}

int sameGender(Request *request) {
  return (curr_gender == 0)
      || (curr_gender == request->gender);
}


int main(int argc, char *argv[]) {
  int rejected_fd;
  int entry_fd;
  Request *request = malloc(sizeof(Request));

  if ( init(argc, argv) )
    exit(1);
  printf("Sauna read the args\n");
  createFifos();
  printf("Sauna created fifos\n");
  if ( openFifos(&rejected_fd, &entry_fd) )
    exit(1);
  printf("Sauna opened fifos\n");
  while( readRequest(request, entry_fd) ) {
    printf("Serial: %lu, Gender: %c, Time: %lu, Rejected: %d\n", request->serial_number, request->gender, request->time_spent, request->times_rejected);

  /*  if( sameGender(request) ) {
      printf("Same gender\n");
      if ( hasSeats() ){
        printf("Has seats\n");
        enter(request);
        printf("Entered\n");
      }
      else {
        putOnHold();
        enter(request);
      }
    } else
      reject(request, rejected_fd);  */

  }
  if ( closeFifos(rejected_fd, entry_fd) )
    exit(1);

  //pthread_exit(NULL);

  return 0;
}
