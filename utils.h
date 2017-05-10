#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define REJECTED_PATH  "/tmp/rejected"
#define ENTRY_PATH     "/tmp/entry"
#define FIFO_MODE      0600

#define BIT(x) (1<<(x))

#define SEND        BIT(0)
#define REJECTED    BIT(1)
#define TREATED     BIT(2)
#define DISCARDED   BIT(3)

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
  char times_rejected;
  char status;
} Request;

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
