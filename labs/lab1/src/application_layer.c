#include "../include/application_layer.h"
#include "../include/application_layer_macros.h"
#include "../include/link_layer.h"
#include "../include/alarm.h"

#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>  // for gettimeofday()
#include <unistd.h>

struct timeval t1, t2;
double elapsedTime;

typedef struct {
  unsigned char type;
  unsigned char length;
  unsigned char* value;
} ctrl_packet_parameter_t;

typedef struct {
  char* file_name;
  int file_descriptor;
  int status;
  unsigned char* file_size;
  int max_size_read;
} application_layer_t;

application_layer_t application_layer;

int openFile() {
  const char* filename = application_layer.file_name;
  
  if (application_layer.status == TRANSMITER) {
    application_layer.file_descriptor =
        open(filename,
             O_RDONLY | O_NOCTTY );  // será que tem que ser fopen?

    if (application_layer.file_descriptor < 0) {
      perror("open file");
      return -1;
    }
  } else {
    application_layer.file_descriptor =
        open(filename,
             O_WRONLY | O_NOCTTY | O_CREAT |O_NONBLOCK);  // será que tem que ser fopen?

    if (application_layer.file_descriptor < 0) {
      perror("open file");
      return -1;
    }
  }

  return 0;
}

int init(char* file_name, char* port) {
  setupAlarm();

  application_layer.max_size_read = 200;
  if (application_layer.status == TRANSMITER) {
    application_layer.file_name = file_name;
    openFile();
  }

  llopen(port, application_layer.status);


  // start timer
  gettimeofday(&t1, NULL);
  return 0;
}

int sendCtrlPacket(unsigned char ctrl) {
  /* assemble file size ctrl packet parameter */
  struct stat st;
  stat(application_layer.file_name, &st);
  off_t fsize = st.st_size;
  unsigned char* packet;

  ctrl_packet_parameter_t file_size_param = {
      .type = PACKET_DATA_FILE_SIZE,
      .length = (int)ceil((float) log(fsize) /8),
      .value = malloc(file_size_param.length * sizeof(unsigned char))};

  if (file_size_param.value == NULL) {
    perror("file_size_param malloc");
    return -1;
  }

  int nth_byte = file_size_param.length - 1;  // starts at msb
  for (int i = 0; i < file_size_param.length; i++) {
    file_size_param.value[i] =
        (fsize >> (nth_byte * 8)) & 0xFF;  // get nth byte
    nth_byte--;
  }

  /* assemble file size ctrl packet parameter */
  size_t fname = strlen(application_layer.file_name) + 1;

  if (fname > 255)
    perror("file_name too big");

  ctrl_packet_parameter_t file_name_param = {
      .type = PACKET_DATA_FILE_NAME,
      .length = (unsigned char)fname,
      .value = malloc(fname * sizeof(unsigned char))};

  if (file_name_param.value == NULL) {
    perror("file_name_param malloc");
    return -1;
  }

  memcpy(file_name_param.value, application_layer.file_name,
         fname);

  int packet_sz = 1 + 2 + 2 + fname + file_size_param.length;
  packet = malloc(packet_sz);

  if (packet == NULL) {
    perror("ctrl packet malloc");
    return -1;
  }

  /* assemble ctrl packet */
  packet[PACKET_CTRL_IX] = ctrl;

  /* TLV for file size parameter */
  packet[PACKET_CTRL_T1_IX] = file_size_param.type;
  packet[PACKET_CTRL_L1_IX] = file_size_param.length;
  memcpy(packet + PACKET_CTRL_V1_IX, file_size_param.value,
         file_size_param.length);

  /* TLV for file name parameter */
  packet[PACKET_CTRL_V1_IX + file_size_param.length] = file_name_param.type;
  packet[PACKET_CTRL_V1_IX + file_size_param.length + 1] =
      file_name_param.length;
  memcpy(packet + PACKET_CTRL_V1_IX + file_size_param.length + 2,
         file_name_param.value, file_name_param.length);
  printf("[Application Layer] Control Packet sent.\n\n");

  return llwrite(packet, packet_sz);
}

int sendDataPacket(unsigned char* data,
                     unsigned char seq_num, int read_sz) {
  unsigned char data_size = read_sz;
  unsigned char packet[4 + data_size];

  if (packet == NULL) {
    perror("data packet malloc");
    return -1;
  }

  packet[PACKET_CTRL_IX] = PACKET_CTRL_DATA;
  packet[PACKET_DATA_SEQ_NUM_IX] = seq_num % PACKET_DATA_SEQ_NUM_SIZE;
  packet[PACKET_DATA_LENGTH_MSB_IX] = (data_size & 0xFF00) >> 8;
  packet[PACKET_DATA_LENGTH_LSB_IX] = data_size & 0x00FF;
  memcpy(packet + PACKET_DATA_START_IX, data, data_size);

  printf("[Application Layer] Packet %d sent.\n\n", seq_num);

  while (llwrite(packet, 4 + data_size) != 0)
    {
    printf("[Application Layer] Packet %d re-sent.\n\n", seq_num);
    }

  return 0;
}

void setID(int id) {
  if (id) {
    application_layer.status = RECEIVER;
  } else {
    application_layer.status = TRANSMITER;
  }
}

int checkCtrlPacket(unsigned char* packet) {

  if (packet[PACKET_CTRL_IX] != PACKET_CTRL_END &&
      packet[PACKET_CTRL_IX] != PACKET_CTRL_START)
    {
      printf("[Application Layer] Packet %d received.\n\n", packet[PACKET_DATA_SEQ_NUM_IX]);
    return 1;
  }

  if (packet[PACKET_CTRL_IX] == PACKET_CTRL_END)
    {
    printf("[Application Layer] Control Packet END received.\n\n");

    return PACKET_CTRL_END;
    }

  int L1 = packet[PACKET_CTRL_L1_IX];
  application_layer.file_size = malloc(L1);
  memcpy(application_layer.file_size, packet + PACKET_CTRL_V1_IX, L1);
  int L2 = packet[PACKET_CTRL_V1_IX + L1 + 2];
  application_layer.file_name = malloc(L2);
  memcpy(application_layer.file_name, packet + PACKET_CTRL_V1_IX + L1 + 2, L2);

  application_layer.file_name = malloc(20);
  application_layer.file_name = "ourFluffyPenguin.gif";
  openFile();

  printf("[Application Layer] Control Packet START received.\n\n");

  return PACKET_CTRL_START;
}

int writeToFile(unsigned char* packet) {
  int L1 = packet[PACKET_DATA_LENGTH_LSB_IX];
  int L2 = packet[PACKET_DATA_LENGTH_MSB_IX];

  int k = (256 * L2) + L1;
  unsigned char file_data[k];

  memcpy(file_data, packet + PACKET_DATA_START_IX, k);

  return write(application_layer.file_descriptor, file_data,
                 sizeof(file_data));
}

int communicate(char* port, char* file_name) {
  init(file_name, port);

  if (application_layer.status == TRANSMITER) {
    // get size file
    unsigned char buf[application_layer.max_size_read];

    sendCtrlPacket(PACKET_CTRL_START);

    int seq_num = 0;
    int c; 
    while ((c = read(application_layer.file_descriptor, buf,
                 application_layer.max_size_read)) > 0) {
      sendDataPacket(buf, seq_num, c);
      seq_num++;
    }

    sendCtrlPacket(PACKET_CTRL_END);
  } else {
    for (;;) {
      unsigned char* packet = llread();

      if (packet == NULL)
        continue;

      int ctrl_byte = checkCtrlPacket(packet);

      if (ctrl_byte == PACKET_CTRL_START)
        continue;
      else if (ctrl_byte == PACKET_CTRL_END)
        break;
      
      writeToFile(packet);
    }
  }

  llclose();
  // stop timer
  gettimeofday(&t2, NULL);

  // compute and print the elapsed time in millisec
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;  // us to ms
  // sec to ms
  printf("%f elapsed time\n", elapsedTime);
  return 0;
}