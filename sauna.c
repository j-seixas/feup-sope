#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define REJECTED_PATH  "/tmp/rejected"
#define ENTRY_PATH     "/tmp/entry"
#define FIFO_MODE      0600

typedef unsigned int uint32;
typedef unsigned long int uint64;

/** @struct Request
 *  @brief Holds a request information
 *
 *  @var Request::serial_number
 *  Holds the serial number of this request
 *
 *  @var Request::time_spent
 *  Indicates the time the user is going to spend inside the sauna
 *
 *  @var Request::gender
 *  Indicates the gender of the user
 */
typedef struct {
  uint64 serial_number;
  uint64 time_spent;
  char gender;
} Request;

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
 *  @brief      Checks if the user can enter the sauna
 *  @param[in]  request              The user request
 *  @param[in]  num_seats_available  The number of seats available in sauna
 *  @param[in]  curr_gender          The current gender of the people inside the sauna
 *  @return     Returns whether or not the user can enter the sauna
 */
int canEnter( Request *request, uint32 *num_seats_available, char *curr_gender ) {
  return (*num_seats_available > 0)
      && (
         *curr_gender == 0
      || *curr_gender == request->gender
    );
}

/**
 *  @brief       Lets a user enter the sauna
 *  @param[in]   request              The accepted user request
 *  @param[out]  num_seats_available  The number of seats available in sauna
 *  @param[out]  curr_gender          The current gender of the people inside the sauna
 */
void enter( Request *request, uint32 *num_seats_available, char *curr_gender ) {
  (*num_seats_available)--;
  *curr_gender = request->gender;
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
int readArgs(uint* num_seats, const int argc, char *argv[]) {
  if ( argc != 2 ) {
    printf("Usage: sauna <num. seats>\n");
    return 1;
  }

  *num_seats = strtol(argv[1],NULL,10);

  return 0;
}

/**
 *  @brief   Creates the entry and rejected fifos
 *  @return  Returns whether or not the fifos were created
 */
int createFifos() {
  int result = 0;
  result |= mkfifo(REJECTED_PATH, FIFO_MODE);
  result |= mkfifo(ENTRY_PATH, FIFO_MODE);
  return result;
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
  return (*rejected_fd == -1 || *entry_fd == -1);
}

/**
 *  @brief      Closes the entry and rejected fifos
 *  @param[in]  rejected_fd  The file descriptor of the rejected fifoo
 *  @param[in]  entry_fd     The file descriptor of the entry fifo
 *  @return     Returns whether or not the fifos were closed
 */
int closeFifos(int rejected_fd, int entry_fd) {
  int result = 0;
  result |= close(rejected_fd);
  result |= close(entry_fd);
  return result;
}

/**
 *  @brief       Processes the leave of a user
 *  @param[in]   request              The accepted user request
 *  @param[out]  num_seats_available  The number of seats available in sauna
 *  @param[out]  curr_gender          The current gender of the people inside the sauna
 *  @param[in]   num_seats            The number of seats of the sauna
 */
void leave( uint32 *num_seats_available, char *curr_gender, const uint32 num_seats) {
  (*num_seats_available)++;
  if(*num_seats_available == num_seats)
    *curr_gender = 0;
}

/**
 *  @brief  Handles the signal sent when a user leaves the sauna
 *  @param  signal  The signal received
 */
void timeupHandler(int signal) {

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
  uint32 num_seats;
  uint32 num_seats_available;
  int rejected_fd;
  int entry_fd;
  char curr_gender = 0;
  Request *request = NULL;

  if ( installTimeupHandler() )
    exit(1);
  if ( readArgs(&num_seats, argc, argv) )
    exit(1);
  if ( createFifos() )
    exit(1);
  if ( openFifos(&rejected_fd, &entry_fd) )
    exit(1);
  num_seats_available = num_seats;
  while( readRequest(request, entry_fd) ) {
    if( canEnter(request, &num_seats_available, &curr_gender) )
      enter(request, &num_seats_available, &curr_gender);
    else
      reject(request, rejected_fd);
  }
  if ( closeFifos(rejected_fd, entry_fd) )
    exit(1);

  return 0;
}
