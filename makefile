CC=gcc
CFLAGS=-Wall -Wextra -pthread -O3
ODIR=./obj

default:all

SAUNA_OBJ=sauna.o
OBJS_PATH=$(patsubst %,$(ODIR)/%,(SAUNA_OBJ))

GENERATOR_OBJ=generator.o
GENERATOR_OBJ=$(patsubst %, $(ODIR)/%,(GENERATOR_OBJ))

sauna:sauna.c
	$(CC) -o $@ $< $(CFLAGS)

generator:generator.c
	$(CC) -o $@ $< $(CFLAGS)


all:sauna generator
	

clean:
	@rm -fr $(ODIR) sauna generator
