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

#define GEN_LOGFILE "/tmp/ger."
#define SAUNA_LOGFILE "/tmp/bal."


typedef unsigned int uint32;
typedef unsigned long int uint64;

/** @struct request_t
 *  @brief Holds a request information
 *
 *  @var request_t::serial_number
 *  Holds the serial number of this request
 *
 *  @var request_t::time_spent
 *  Indicates the time the user is going to spend inside the sauna
 *
 *  @var request_t::gender
 *  Indicates the gender of the user
 *
 *  @var request_t::times_rejected
 *  How many times the request was rejected
 *
 *  @var request_t::status
 *  Current status of the request
*/
typedef struct {
  uint64 serial_number;
  uint64 time_spent;
  char gender;
  char times_rejected;
  char status;
} request_t;

/**
 * @struct gen_log_t
 * @brief Stores the information to be printed to the generator log file
 * 
 * @var gen_log_t::inst
 * Instant when this log was generated
 *
 * @var gen_log_t::pid
 * ID of the process which generated this log
 *
 * @var gen_log_t::p
 * Number of the request
 *
 * @var gen_log_t::g
 * Gender of the request
 *
 * @var gen_log_t::dur
 * Duration of the request
 *
 * @var gen_log_t::tip
 * The identifier of the message
 */
typedef struct {
  double inst;
  int pid;
  int p;
  char g;
  uint64 dur;
  char *tip;
}gen_log_t;


/**
 * @struct sauna_log_t
 * @brief Stores the information to be printed to the generator log file
 * 
 * @var sauna_log_t::inst
 * Instant when this log was generated
 *
 * @var sauna_log_t::pid
 * ID of the process which generated this log
 *
 * @var sauna_log_t::tid
 * ID of the thread that generated this log
 *
 * @var sauna_log_t::p
 * Number of the request
 *
 * @var sauna_log_t::g
 * Gender of the request
 *
 * @var sauna_log_t::dur
 * Duration of the request
 *
 * @var sauna_log_t::tip
 * The identifier of the message
 */
typedef struct {
  double inst;
  int pid;
  int tid;
  int p;
  char g;
  uint64 dur;
  char *tip;
}sauna_log_t;


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

/**
 * @brief Opens the designated log file
 * @param[in] Path to the file to open
 * @return Returns what the open system call returns
 * @detail It appends the pid of the process to the pathname before opening the file
 */
int openLogFile(char * pathname){
  char pid[100], *path_name = (char*)malloc(sizeof(char)*(strlen(pathname)+100));
  memcpy(path_name, pathname,strlen(pathname));
  sprintf(pid,"%d",getpid());
  strcat(path_name,pid);
  return open(path_name, O_CREAT | O_WRONLY);
}


