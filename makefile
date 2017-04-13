CC=gcc
CFLAGS=-Wall -Wextra -O3
ODIR=./obj

default:sfind

OBJS_NAME=sfind.o
OBJS_PATH=$(patsubst %,$(ODIR)/%,$(OBJS_NAME))


$(OBJS_PATH): sfind.c
	@mkdir -p obj
	@$(CC) -c -o $@ $< $(CFLAGS)

sfind: $(OBJS_PATH)
	@$(CC) -o $@ $^ $(CFLAGS)

clean:
	@rm -fr $(ODIR) sfind