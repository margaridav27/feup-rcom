#include "../include/application_layer.h"
#include <fcntl.h>
#include <stdio.h>
#include "../include/physical_layer.h"

typedef struct {
  int fileDescriptor;
  int status;
} application_layer_t;

application_layer_t application_layer;

int llopen(char* filename, char* port) {
  int fd_file = open(filename, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_file < 0) {
    perror("open file");
    return -1;
  }

  /* setup serial port & establish communication (send SET ctrl frame) */
  setPhysicalLayer(port);
}

// TODO #1
int createPacket() {
  return -1;
}

// TODO #2
int emmit(char* filename, char* port) {
  llopen(filename, port);

  // loop reading file
  /* read part of file
     create packet
     send to physical layer
  */

  // send control packet again when finished

  llclose(filename, port);
  return 0;
}

void llclose(char* filename, char* port) {
  close(filename);

  /* end communication by
    1. sending DISC ctrl frame
    2. waiting for DISC ctrl frame that will be sent by receiver
    3. sending UA ctrl frame and effectively ending communication 
  */
  termination();
}