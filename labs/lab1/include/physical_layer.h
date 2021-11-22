#ifndef PHYSICAL_LAYER_H
#define PHYSICAL_LAYER_H

#define MAX_SIZE 100

typedef enum { TRANSMITER, RECEIVER } flag_t;

typedef struct {
  char port[20];
  int fd;
  int baud_rate;
  unsigned char sequence_num;
  unsigned int timeout;
  unsigned int num_transmissions;
  char frame[MAX_SIZE];
  flag_t status;
} link_layer_t;

int llopen(char* port, flag_t flag);

int llclose();

unsigned char getBCC2(unsigned char* data, int packet_sz);

int packetToFrame(unsigned char* packet, unsigned char* frame, int packet_sz);

void stuffing(unsigned char* frame, int frame_sz);

void destuffing(unsigned char* frame, int frame_sz);

int llwrite(unsigned char* packet, int packet_sz);

unsigned char* llread();

void setupLinkLayer();

void assembleCtrlFrame(unsigned char addr,
                       unsigned char ctrl,
                       unsigned char* frame);

int writeFrame(unsigned char* frame, int sz);

int readFrame(unsigned char* frame, int sz);

enum state_t validateCtrlFrame(unsigned char addr,
                               unsigned char ctrl,
                               unsigned char* frame,
                               enum state_t curr_state);

enum state_t validateIFrame(unsigned char addr,
                            unsigned char* frame,
                            enum state_t curr_state);

int establishmentTransmitter();

int establishmentReceiver();

int terminationTransmitter();

int terminationReceiver();

#endif