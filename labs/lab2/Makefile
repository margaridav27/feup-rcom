#c compiler
CC := cc

CFLAGS := -Wall -o

BIN := bin/
SRC := src/
INCLUDE := include/

all:  download 

download: ${INCLUDE}parser.h ${INCLUDE}commands.h ${INCLUDE}tcp.h ${SRC}download.c 
	$(CC) $(CFLAGS) ${BIN}download  ${SRC}parser.c ${SRC}commands.c ${SRC}tcp.c  ${SRC}download.c 

clean:
	rm -f download  