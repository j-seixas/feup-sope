#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define REJECTED_PATH  "/tmp/rejected"
#define ENTRY_PATH     "/tmp/entry"
#define FIFO_MODE      0600

typedef unsigned int uint;

typedef struct {
  uint serial_number;
  uint time_spent;
  char gender;
} Request;

int readRequest( Request *request, int entry_fd ) {

}

int canEnter( Request *request, uint *num_seats_available, char *gender ) {

}

int enter( Request *request, uint *num_seats_available, char *gender ) {

}

int reject( Request *request, int rejected_fd ) {

}

int readArgs(uint* num_seats, uint* time_multiplier, const int argc, char *argv[]) {
  if ( argc != 3 ) {
    printf("Usage: sauna <num. seats> <time unit>\n");
    return 1;
  }

  *num_seats = strtol(argv[1],NULL,10);
  char time_unit = argv[2][0];

  if ( time_unit == 's' )
    *time_multiplier = 1000000;
  else if ( time_unit == 'm' )
    *time_multiplier = 1000;
  else if ( time_unit == 'u' )
    *time_multiplier = 1;
  else return 1;

  return 0;
}

int createFifos() {
  int result = 0;
  result |= mkfifo(REJECTED_PATH, FIFO_MODE);
  result |= mkfifo(ENTRY_PATH, FIFO_MODE);
  return result;
}

int openFifos(int *rejected_fd, int *entry_fd) {
  *rejected_fd = open(REJECTED_PATH, O_WRONLY);
  *entry_fd = open(ENTRY_PATH, O_RDONLY);
  return (*rejected_fd == -1 || *entry_fd == -1);
}

int closeFifos(int *rejected_fd, int *entry_fd) {
  int result = 0;
  result |= close(*rejected_fd);
  result |= close(*entry_fd);
  return result;
}

int main(int argc, char *argv[]) {
  uint num_seats;
  uint num_seats_available;
  uint time_multiplier;
  int rejected_fd;
  int entry_fd;
  char curr_gender = 0;
  Request *request;

  if ( readArgs(&num_seats, &time_multiplier, argc, argv) )
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
  if ( closeFifos(&rejected_fd, &entry_fd) )
    exit(1);

  return 0;
}
