#include <stdio.h>
#include <stdlib.h>

#define INVALID_ARGS 1

int main ( int argc , char *argv[] ) {

  if ( argc != 3 ) {
		printf("Usage: generator <num. seats> <time unit>\n");
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
