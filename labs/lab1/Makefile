#c compiler
CC := cc


CFLAGS := -Wall -o
all:  emitter receiver


emitter: emitter.c 
	$(CC) $(CFLAGS)  emitter emitter.c

receiver: receiver.c 
	$(CC)  $(CFLAGS) receiver receiver.c

clean:
	rm -f receiver emissor  