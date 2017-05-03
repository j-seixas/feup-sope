#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define INVALID_ARGS 1

int main ( int argc , char *argv[] ) {

  if ( argc != 3 ) {
		write(STDOUT_FILENO,"Usage: sauna <num. seats> <time unit>\n",42);
		exit(INVALID_ARGS);
	}

	int  num_seats = strtol(argv[1],NULL,10);
  int  time_multiplier;
  char time_unit = argv[2][0];

  if ( time_unit == 's' )
    time_multiplier = 1000000;
  else if ( time_unit == 'm' )
    time_multiplier = 1000;
  else if ( time_unit == 'u' )
    time_multiplier = 1;
  else exit(INVALID_ARGS);

}
