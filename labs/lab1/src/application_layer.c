#include "../include/application_layer.h"

#include <fcntl.h>
#include <stdio.h>

typedef struct {
  int fileDescriptor;
  int status;
} apllication_layer_t;

int llopen(char* filename) {
  int fd = open(filename, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    perror("open");
    return -1;
  }
}

void llclose(char* fileName) {
  close(fileName);
}