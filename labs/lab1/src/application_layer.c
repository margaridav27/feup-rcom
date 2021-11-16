#include "../include/application_layer.h"
#include "../include/application_layer_macros.h"
#include "../include/physical_layer.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

typedef struct {
  char type;
  char length;
  char* value;
} ctrl_packet_parameter_t;

typedef struct {
  char* file_name;
  int file_descriptor;
  int status;
  int file_portion;
} application_layer_t;

application_layer_t application_layer;

int init(char* file_name, char* port) {
  application_layer.file_name = file_name;
  application_layer.file_descriptor =
      open(file_name,
           O_RDWR | O_NOCTTY | O_NONBLOCK);  // será que tem que ser fopen?

  if (application_layer.file_descriptor < 0) {
    perror("open file");
    return -1;
  }

  llopen(port, TRANSMITER);
}

int createCtrlPacket(char ctrl, char* packet) {
  struct stat st;
  stat(application_layer.file_name, &st);

  off_t size = st.st_size;

  ctrl_packet_parameter_t file_size = {
      .type = DATA_FILE_SIZE,
      .length = OCTET_SIZE % size + 1,
      .value = malloc(file_size.length * sizeof(char))};

  int nth_byte = file_size.length - 1;  // starts at msb
  for (int i = 0; i < file_size.length; i++) {
    file_size.value[i] = (size >> (nth_byte * 8)) & 0xFF;  // get nth byte
    nth_byte--;
  }

  size_t name_size = strlen(application_layer.file_name) + 1;

  ctrl_packet_parameter_t file_name = {
      .type = DATA_FILE_NAME,
      .length = OCTET_SIZE % name_size + 1,
      .value = malloc(file_name.length * sizeof(char))};

  int nth_byte = file_name.length - 1;  // starts at msb
  for (int i = 0; i < file_name.length; i++) {
    file_name.value[i] = application_layer.file_name[i];
    nth_byte--;
  }

  packet = malloc(1 + sizeof(file_size) + sizeof(file_name));
  // TODO preencher efetivamente o packet com os values

  packet[CTRL_IX] = ctrl;

  return 0;
}

// TODO #1
int createDataPacket() {
  return -1;
}

// TODO #2
int emmit(char* file_name, char* port) {
  init(file_name, port);

  // get size file
  unsigned char buf[application_layer.file_portion],
      packet[application_layer.file_portion * 2];

  createCtrlPacket(CTRL_START, packet);
  llwrite(packet);

  while (read(application_layer.file_descriptor, buf,
              application_layer.file_portion) > 0) {
    createDataPacket(buf, packet);

    llwrite(packet);
  }

  createCtrlPacket(CTRL_END, packet);
  llwrite(packet);

  llclose(file_name, port);
  return 0;
}

void end(char* file_name, char* port) {
  llclose(file_name);

  /* end communication by
    1. sending DISC ctrl frame
    2. waiting for DISC ctrl frame that will be sent by receiver
    3. sending UA ctrl frame and effectively ending communication
  */
  termination();
}