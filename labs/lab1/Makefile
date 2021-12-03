#c compiler
CC := cc

CFLAGS := -Wall -o

BIN := bin/
SRC := src/
INCLUDE := include/

all:  transmitter receiver

transmitter: ${INCLUDE}alarm.h ${INCLUDE}application_layer.h ${INCLUDE}link_layer.h ${INCLUDE}serial_port.h ${SRC}transmitter.c 
	$(CC) $(CFLAGS) ${BIN}transmitter  ${SRC}alarm.c ${SRC}application_layer.c ${SRC}link_layer.c ${SRC}serial_port.c ${SRC}transmitter.c -lm

receiver: ${INCLUDE}alarm.h ${INCLUDE}application_layer.h ${INCLUDE}link_layer.h ${INCLUDE}serial_port.h  ${SRC}receiver.c 
	$(CC) $(CFLAGS) ${BIN}receiver  ${SRC}alarm.c ${SRC}application_layer.c ${SRC}link_layer.c ${SRC}serial_port.c ${SRC}receiver.c -lm

clean:
	rm -f receiver transmitter 