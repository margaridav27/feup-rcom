#ifndef PHYSICAL_LAYER_H
#define PHYSICAL_LAYER_H

#define MAX_SIZE 100

typedef struct {
  char port[20];
  int fd;
  int baud_rate;
  unsigned int sequence_num;
  unsigned int timeout;
  unsigned int num_transmissions;
  char frame[MAX_SIZE];
} link_layer_t;

typedef enum { TRANSMITER, RECEIVER } flag_t;

int llopen(char* port, flag_t flag);

int llclose();

int llwrite(char* packet);

int llread();

void setupLinkLayer();

void assembleCtrlFrame(char addr, char ctrl, char** frame);

int writeCtrlFrame(char* frame);

int readCtrlFrame(char* frame);

enum state_t validateCtrlFrame(char addr,
                               char ctrl,
                               char* frame,
                               enum state_t curr_state);

int establishment();

int termination();

int packetToFrame(char* packet, char* frame);

int stuffing(char* frame);

int destuffing(char* frame);

#endif