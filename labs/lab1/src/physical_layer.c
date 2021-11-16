#include "../include/physical_layer.h"
#include <fcntl.h>
#include "../include/macros.h"

enum state_t { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP };

link_layer_t link_layer;
extern int flag, try;

void setupLinkLayer() {
  link_layer.baud_rate = BAUDRATE;
  link_layer.sequence_num = 0;
  link_layer.timeout = 1;
  link_layer.num_transmissions = 3;
}

int assembleCtrlFrame(char addr, char ctrl, char** frame) {
  frame[0] = FLAG_BYTE;
  frame[1] = ADDR_CR_RE;
  frame[2] = ctrl;
  frame[3] = BCC1(addr, ctrl);
  frame[4] = FLAG_BYTE;
}

int writeCtrlFrame(char* frame) {
  return write(link_layer.port, frame, 5);
}

int readCtrlFrame(char* frame) {
  read(link_layer.port, frame, 1);
}

int establishment() {
  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  char set_cmd[5], response[1];

  /* assemble SET frame */
  assembleCtrlFrame(ADDR_CE_RR, CTRL_SET, set_cmd);

  while (msg_state != STOP && try < 4) {
    if (flag) {
      /* send SET frame to receiver */
      num_bytes_written = writeCtrlFrame(set_cmd);

      printf("try #%d: %x %x %x %x %x (%d bytes)\n", try, set_cmd[0],
             set_cmd[1], set_cmd[2], set_cmd[3], set_cmd[4], num_bytes_written);

      alarm(3);
      msg_state = START;
    }

    flag = 0;

    /* read UA frame sent by receiver */
    num_bytes_read = readCtrlFrame(response);

    /* validate UA frame sent by receiver */
    switch (msg_state) {
      case START:
        if (response[0] == FLAG_BYTE)
          msg_state = FLAG_RCV;
        break;
      case FLAG_RCV:
        if (response[0] == ADDR_CE_RR)
          msg_state = A_RCV;
        else
          msg_state = START;
        break;
      case A_RCV:
        if (response[0] == CTRL_UA)
          msg_state = C_RCV;
        else if (response[0] == FLAG_BYTE)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case C_RCV:
        if (response[0] == BCC1(ADDR_CE_RR, CTRL_UA))
          msg_state = BCC_OK;
        else if (response[0] == FLAG_BYTE)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case BCC_OK:
        if (response[0] == FLAG_BYTE)
          msg_state = STOP;
        else
          msg_state = START;
        break;
      default:
        msg_state = START;
        break;
    }
  }

  if (msg_state == STOP) {
    printf("\nUA received.\n");
    return 0;
  } else {
    printf("\nUA not received.\n");
    return -1;
  }
}

int acknowledgment() {
  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  char set_cmd[1], ua_cmd[5];

  while (msg_state != STOP) {
    /* read SET frame sent by emitter */
    num_bytes_read = readCtrlFrame(set_cmd);

    if (num_bytes_read > 0)
      printf("%x \n", set_cmd[0]);

    /* validate SET frame sent by emitter */
    switch (msg_state) {
      case START:
        if (set_cmd[0] == FLAG_BYTE)
          msg_state = FLAG_RCV;
        break;
      case FLAG_RCV:
        if (set_cmd[0] == ADDR_CE_RR)
          msg_state = A_RCV;
        else
          msg_state = START;
        break;
      case A_RCV:
        if (set_cmd[0] == CTRL_SET)
          msg_state = C_RCV;
        else if (set_cmd[0] == FLAG_BYTE)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case C_RCV:
        if (set_cmd[0] == BCC1(ADDR_CE_RR, CTRL_SET))
          msg_state = BCC_OK;
        else if (set_cmd[0] == FLAG_BYTE)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case BCC_OK:
        if (set_cmd[0] == FLAG_BYTE)
          msg_state = STOP;
        else
          msg_state = START;
        break;
      default:
        msg_state = START;
        break;
    }
  }

  printf("\n");

  /* assemble UA frame */
  ua_cmd[0] = FLAG_BYTE;
  ua_cmd[1] = ADDR_CR_RE;
  ua_cmd[2] = CTRL_UA;
  ua_cmd[3] = BCC1(ADDR_CR_RE, CTRL_UA);
  ua_cmd[4] = FLAG_BYTE;

  /* send UA frame to emitter */
  num_bytes_written = writeCtrlFrame(ua_cmd);
  printf("Sent to emissor: %x %x %x %x %x (%d bytes)\n", ua_cmd[0], ua_cmd[1],
         ua_cmd[2], ua_cmd[3], ua_cmd[4], num_bytes_written);
}

int termination() {
  /* end communication by
    1. sending DISC ctrl frame
    2. waiting for DISC ctrl frame that will be sent by receiver
    3. sending UA ctrl frame and effectively ending communication
  */

  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  char set_cmd[5], response[1];

  assembleCtrlFrame(ADDR_CE_RR, CTRL_DISC, set_cmd);

  while (msg_state != STOP && try < 4) {
    if (flag) {
      num_bytes_written = writeCtrlFrame(set_cmd);

      printf("try #%d: %x %x %x %x %x (%d bytes)\n", try, set_cmd[0],
             set_cmd[1], set_cmd[2], set_cmd[3], set_cmd[4], num_bytes_written);

      alarm(3);
      msg_state = START;
    }

    flag = 0;

    num_bytes_read = readCtrlFrame(response);

    switch (msg_state) {
      case START:
        if (response[0] == FLAG_BYTE)
          msg_state = FLAG_RCV;
        break;
      case FLAG_RCV:
        if (response[0] == ADDR_CE_RR)
          msg_state = A_RCV;
        else
          msg_state = START;
        break;
      case A_RCV:
        if (response[0] == CTRL_DISC)
          msg_state = C_RCV;
        else if (response[0] == FLAG_BYTE)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case C_RCV:
        if (response[0] == BCC1(ADDR_CE_RR, CTRL_DISC))
          msg_state = BCC_OK;
        else if (response[0] == FLAG_BYTE)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case BCC_OK:
        if (response[0] == FLAG_BYTE)
          msg_state = STOP;
        else
          msg_state = START;
        break;
      default:
        msg_state = START;
        break;
    }
  }

  if (msg_state != STOP)
    return -1;

  assembleCtrlFrame(ADDR_CE_RR, CTRL_UA, set_cmd);
  num_bytes_written = writeCtrlFrame(set_cmd);

  printf("Sent final UA to receiver: %x %x %x %x %x (%d bytes)\n", set_cmd[0],
         set_cmd[1], set_cmd[2], set_cmd[3], set_cmd[4], num_bytes_written);

  resetSerialPort();

  close(link_layer.port);
};

int setPhysicalLayer(char* port) {
  int fd_port = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_port < 0) {
    perror("open port");
    return -1;
  }

  /* setup serial port */
  setupSerialPort(port);

  /* establish connection by sending SET ctrl frame */
  establishment();
}

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
