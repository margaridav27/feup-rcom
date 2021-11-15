#ifndef PHYSICAL_LAYER_H
#define PHYSICAL_LAYER_H

#define MAX_SIZE 100

typedef struct {
  char port[20];
  int baud_rate;
  unsigned int sequence_num;
  unsigned int timeout;
  unsigned int num_transmissions;
  char frame[MAX_SIZE];
} link_layer_t;

void setupLinkLayer();

int establishment();

int createCommandFrame(unsigned char control, char** cmd);

#endif