#c compiler
CC := cc

CFLAGS := -Wall -o

BIN := bin/
SRC := src/
INCLUDE := include/

all:  emitter receiver

emitter: ${INCLUDE}alarm.h ${SRC}emitter.c 
	$(CC) $(CFLAGS) ${BIN}emitter ${SRC}alarm.c ${SRC}emitter.c

receiver: ${SRC}receiver.c 
	$(CC) $(CFLAGS) ${BIN}receiver ${SRC}receiver.c

clean:
	rm -f receiver emissor 