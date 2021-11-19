#include "../include/application_layer.h"
#include "../include/application_layer_macros.h"
#include "../include/physical_layer.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct
{
  unsigned char type;
  unsigned char length;
  unsigned char* value;
} ctrl_packet_parameter_t;

typedef struct
{
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
           O_RDWR | O_NOCTTY | O_NONBLOCK); // será que tem que ser fopen?

  if (application_layer.file_descriptor < 0)
  {
    perror("open file");
    return -1;
  }

  llopen(port, application_layer.status);
  return 0;
}

int createCtrlPacket(unsigned char ctrl, unsigned char* packet) {
  struct stat st;
  stat(application_layer.file_name, &st);

  off_t size = st.st_size;

  ctrl_packet_parameter_t file_size = {
      .type = DATA_FILE_SIZE,
      .length = OCTET_SIZE % size + 1,
      .value = malloc(file_size.length * sizeof(unsigned char))};

  if (file_size.value == NULL)
  {
    perror("file_size malloc");
    return -1;
  }

  int nth_byte = file_size.length - 1; // starts at msb
  for (int i = 0; i < file_size.length; i++)
  {
    file_size.value[i] = (size >> (nth_byte * 8)) & 0xFF; // get nth byte
    nth_byte--;
  }

  size_t name_size = strlen(application_layer.file_name) + 1;

  ctrl_packet_parameter_t file_name = {
      .type = DATA_FILE_NAME,
      .length = OCTET_SIZE % name_size + 1,
      .value = malloc(file_name.length * sizeof(unsigned char))};

  if (file_name.value == NULL)
  {
    perror("file_name malloc");
    return -1;
  }

  nth_byte = file_name.length - 1; // starts at msb
  for (int i = 0; i < file_name.length; i++)
  {
    file_name.value[i] = application_layer.file_name[i];
    nth_byte--;
  }

  packet = malloc(1 + sizeof(file_size) + sizeof(file_name));

  if (packet == NULL)
  {
    perror("ctrl packet malloc");
    return -1;
  }

  packet[CTRL_IX] = ctrl;
  // TODO preencher efetivamente o packet com os values

  return 0;
}

int createDataPacket(unsigned char* data,
                     unsigned char seq_num,
                     unsigned char* packet) {
  unsigned char data_size = sizeof(data);

  packet = malloc(4 + data_size);
  if (packet == NULL)
  {
    perror("data packet malloc");
    return -1;
  }

  packet[CTRL_IX] = CTRL_DATA;
  packet[DATA_SEQ_NUM_IX] = seq_num % DATA_SEQ_NUM_SIZE;
  packet[DATA_LENGTH_MSB_IX] = (data_size & 0xFF00) >> 8;
  packet[DATA_LENGTH_LSB_IX] = data_size & 0x00FF;
  memcpy(packet + DATA_START_IX, data, data_size);

  return 0;
}

void setID (int id) {
  if (id) {
    application_layer.status = RECEIVER;
  } else {
    application_layer.status = TRANSMITER;
  }
}

// TODO #2
int communicate(char* port, char* file_name) {
  init(file_name, port);


  if (application_layer.status == TRANSMITER)
  {
    // get size file
    unsigned char buf[application_layer.file_portion],
        packet[application_layer.file_portion * 2];

    createCtrlPacket(CTRL_START, packet);
    llwrite(packet);

    unsigned char seq_num = 0;
    while (read(application_layer.file_descriptor, buf,
                application_layer.file_portion) > 0)
    {
      createDataPacket(buf, seq_num, packet);
      seq_num++;

      llwrite(packet);
    }

    createCtrlPacket(CTRL_END, packet);
    llwrite(packet);
  } else {
  }
  */

  llclose();
  return 0;
}

void end(unsigned char* file_name, unsigned char* port) {
  llclose(file_name);

  /* end communication by
    1. sending DISC ctrl frame
    2. waiting for DISC ctrl frame that will be sent by receiver
    3. sending UA ctrl frame and effectively ending communication
  */
}
