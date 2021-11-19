#include "../include/physical_layer.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/physical_layer_macros.h"
#include "../include/serial_port.h"

enum state_t { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP };

link_layer_t link_layer;
extern int flag, try;

int llopen(char* port, flag_t flag) {
  link_layer.status = flag;

  if (link_layer.status == TRANSMITER) {
    link_layer.fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (link_layer.fd < 0) {
      perror("llopen");
      return -1;
    }
  } else {
    link_layer.fd = open(port, O_RDWR | O_NOCTTY);
    if (link_layer.fd < 0) {
      perror("llopen");
      return -1;
    }
  }

  /* setup serial port */
  setupLinkLayer();
  setupSerialPort(link_layer.fd);

  /* establish connection by sending SET ctrl frame */
  if (flag == TRANSMITER) {
    establishmentTransmitter();
  } else {
    establishmentReceiver();
    // receiver a receber estabelishment();
  }
  return 0;
}

int llclose() {
  if (link_layer.status == TRANSMITER) {
    terminationTransmitter();
  } else {
    terminationReceiver();
  }

  resetSerialPort();
  close(link_layer.fd);
  return 0;
}

int packetToFrame(unsigned char* packet, unsigned char* frame) {
    size_t packet_sz = sizeof(*packet);
    frame = malloc (packet_sz * sizeof(unsigned char));
     frame[0] = FLAG_BYTE;
     frame[1] = ADDR_CE_RR;
     frame[2] = link_layer.sequence_number;
     frame[3] = BCC1(frame[1], frame[2]);
     memcpy(frame[4], packet, packet_sz);

     int BCC2 = packet[0];
     for (int i = 1; i < packet_sz; i++) {
        BCC2 = BCC2 ^ packet[i];
     }

     frame[4+packet_sz] = BCC2;
     frame[5+packet_sz] = FLAG_BYTE;
}

int stuffing(char* frame) {
    size_t frame_sz = sizeof(*frame);
    char* stuffed_frame = malloc(2*frame_sz);
    int stuffed_frame_ix = 0;

    for (int i = 0; i < frame_sz; i++){
        if (frame[i] == FLAG_BYTE){
            stuffed_frame[stuffed_frame_ix] = ESCAPE_BYTE;
            stuffed_frame_ix++;
            stuffed_frame[stuffed_frame_ix] = FLAG_STUFFING_BYTE;
            }
        else if (frame[i] == ESCAPE_BYTE) {
            stuffed_frame[stuffed_frame_ix] = frame[i];
            stuffed_frame_ix++;
            stuffed_frame[stuffed_frame_ix] = ESCAPE_STUFFING_BYTE;
        } else {
            stuffed_frame[stuffed_frame_ix] = frame[i];
        }
        stuffed_frame_ix++;
    }

    memcpy(frame, stuffed_frame, sizeof(*stuffed_frame));
}

// TODO #11
int llwrite(unsigned char* packet) {
    char* frame;
     packetToFrame(packet, frame);
     stuffing(frame);
     writeFrame(frame);

  return 0;
}

int llread() {
  return 0;
}

void setupLinkLayer() {
  link_layer.baud_rate = BAUDRATE;
  link_layer.sequence_num = 0;
  link_layer.timeout = 1;
  link_layer.num_transmissions = 3;
}

void assembleCtrlFrame(unsigned char addr,
                       unsigned char ctrl,
                       unsigned char* frame) {
  frame[0] = FLAG_BYTE;
  frame[1] = addr;
  frame[2] = ctrl;
  frame[3] = BCC1(addr, ctrl);
  frame[4] = FLAG_BYTE;
}

int writeFrame(unsigned char* frame) {
  return write(link_layer.fd, frame, sizeof(*frame);
}

int readFrame(unsigned char* frame, int sz) {
  return read(link_layer.fd, frame, sz);
}

enum state_t validateCtrlFrame(unsigned char addr,
                               unsigned char ctrl,
                               unsigned char* frame,
                               enum state_t curr_state) {
  enum state_t next_state = START;

  switch (curr_state) {
    case START:
      if (frame[0] == FLAG_BYTE) 
        next_state = FLAG_RCV;
      break;
    case FLAG_RCV:
      if (frame[0] == addr) 
        next_state = A_RCV;
      else 
        next_state = START;
      break;
    case A_RCV:
      if (frame[0] == ctrl) 
        next_state = C_RCV;
      else if (frame[0] == FLAG_BYTE) 
        next_state = FLAG_RCV;
      else 
        next_state = FLAG_RCV;
      break;
    case C_RCV:
      if (frame[0] == BCC1(addr, ctrl)) 
        next_state = BCC_OK;
      else if (frame[0] == FLAG_BYTE) 
        next_state = FLAG_RCV;
      else 
        next_state = START;
      break;
    case BCC_OK:
      if (frame[0] == FLAG_BYTE) 
        next_state = STOP;
      else 
        next_state = START;
      break;
    default:
      next_state = START;
      break;
  }

  return next_state;
}

int establishmentTransmitter() {
  try = 1;

  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  unsigned char set[5], response[1];

  assembleCtrlFrame(ADDR_CE_RR, CTRL_SET, set);

  while (msg_state != STOP && try < 4) {
    if (flag && msg_state != STOP) {
      num_bytes_written = writeFrame(set);

      printf("Try SET #%d: %x %x %x %x %x (%d bytes written)\n\n", try, set[0],
             set[1], set[2], set[3], set[4], num_bytes_written);

      alarm(6);
      msg_state = START;
    }

    flag = 0;

    num_bytes_read = readCtrlFrame(response);
    if (num_bytes_read > 0){
      msg_state = validateCtrlFrame(ADDR_CR_RE, CTRL_UA, response, msg_state);
    }
  }

  if (msg_state == STOP) {
    printf("UA received from transmitted.\n\n");
    return 0;
  } else {
    printf("UA not received from transmitted.\n\n");
    return -1;
  }
}

int establishmentReceiver() {
  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  unsigned char set_cmd[1], ua_cmd[5];

  while (msg_state != STOP) {
    num_bytes_read = readCtrlFrame(set_cmd);

    if (num_bytes_read > 0)
    msg_state = validateCtrlFrame(ADDR_CE_RR, CTRL_SET, set_cmd, msg_state);
  }

  if (msg_state != STOP) {
    printf("Receiver did not send DISC frame back to transmitter.\n\n");
    return -1;
  } else {
    printf("SET frame received from transmitter.\n\n");
  }

  assembleCtrlFrame(ADDR_CR_RE, CTRL_UA, ua_cmd);

  num_bytes_written = writeFrame(ua_cmd);
  printf("Sent UA to transmitter: %x %x %x %x %x (%d bytes written)\n\n", ua_cmd[0],
         ua_cmd[1], ua_cmd[2], ua_cmd[3], ua_cmd[4], num_bytes_written);
  return 0;
}

int terminationTransmitter() {
  try = 1;
  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  unsigned char disc[5], ua[5], response[1];

  assembleCtrlFrame(ADDR_CE_RR, CTRL_DISC, disc);

  while (msg_state != STOP && try < 4) {
    if (flag) {
      num_bytes_written = writeFrame(disc);

      printf("Try DISC #%d: %x %x %x %x %x (%d bytes written)\n\n", try, disc[0],
             disc[1], disc[2], disc[3], disc[4], num_bytes_written);

      alarm(3);
      msg_state = START;
    }

    flag = 0;

    num_bytes_read = readCtrlFrame(response);
    if (num_bytes_read > 0) {
      msg_state = validateCtrlFrame(ADDR_CR_RE, CTRL_DISC, response, msg_state);
    }
  }

  if (msg_state != STOP) {
    printf("Receiver did not send DISC frame back to transmitter.\n\n");
    return -1;
  } else {
    printf("DISC received back from receiver.\n\n");
  }

  assembleCtrlFrame(ADDR_CE_RR, CTRL_UA, ua);

  num_bytes_written = writeFrame(ua);

  printf("Sent final UA to transmitter: %x %x %x %x %x (%d bytes written)\n\n",
         ua[0], ua[1], ua[2], ua[3], ua[4], num_bytes_written);
         
  return 0;
}

int terminationReceiver() {
  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  unsigned char receive_cmd[1], disc_cmd[5];

  while (msg_state != STOP) {
    num_bytes_read = readCtrlFrame(receive_cmd);

    if (num_bytes_read > 0) {
    msg_state =
        validateCtrlFrame(ADDR_CE_RR, CTRL_DISC, receive_cmd, msg_state);
    }
  }

  if (msg_state != STOP) {
    printf("Receiver did not receive DISC frame back from transmitter.\n\n");
    return -1;
  }
  else {
    printf("DISC frame received from transmitter.\n\n");
  }

  assembleCtrlFrame(ADDR_CR_RE, CTRL_DISC, disc_cmd);

  num_bytes_written = writeFrame(disc_cmd);
  printf("Sent DISC back to emissor: %x %x %x %x %x (%d bytes written)\n\n", disc_cmd[0],
         disc_cmd[1], disc_cmd[2], disc_cmd[3], disc_cmd[4], num_bytes_written);

  msg_state = START;

  while (msg_state != STOP) {
    num_bytes_read = readCtrlFrame(receive_cmd);

    if (num_bytes_read > 0)
    msg_state =
        validateCtrlFrame(ADDR_CE_RR, CTRL_UA, receive_cmd, msg_state);
  }

  if (msg_state != STOP) {
    printf("UA frame didn't received back from transmitter.\n\n");
    return -1;
  }
  else {
    printf("UA received from transmitter.\n");
  }
  return 0;
}

// TODO #8
int packetToFrame(unsigned char* packet, unsigned char* frame) {
  // frame = FLAG | campo de endereçamento | campo de controlo | BCC1 | DADOS
  // | BCC2 | FLAG; pacote = Campo de controlo (1- dados) | N numero de
  // sequencia modulo 255 | L2 | L1 | P1 | ... | PK;  K = 256* L2 + L1
  return 0;
}

// TODO #9
int stuffing(unsigned char* frame) {
  return 0;
}

// TODO #10
int destuffing(unsigned char* frame) {
  return 0;
}

/*
int receive(char* filename, char* port) {
  llopen(filename, port);

  // wait for ctrl set frame
  // send ctrl set frame to application layer for it to execute llopen

  // wait for first frame - special frame

  // loop
  // read i-frame
  // send rr or rej ctrl frame
  // create packet
  // send packet to application layer
  // if i-frame == disc ctrl frame then break

  llclose(filename, port);
  return;
}
*/
