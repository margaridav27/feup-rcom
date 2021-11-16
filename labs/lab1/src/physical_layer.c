#include "../include/physical_layer.h"
#include "../include/physical_layer_macros.h"

#include <fcntl.h>

enum state_t { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP };

link_layer_t link_layer;
extern int flag, try;

int llopen(char* port, flag_t flag) {
  link_layer.fd =
      open(port, (O_RDWR && flag) | O_NOCTTY | O_NONBLOCK | (O_WRONLY && flag));
  if (link_layer.fd < 0) {
    perror("llopen");
    return -1;
  }

  /* setup serial port */
  setupSerialPort(port);

  /* establish connection by sending SET ctrl frame */
  establishment();
}

int llclose() {
  termination();

  resetSerialPort();
  close(link_layer.fd);
}

// TODO #11
int llwrite(char* packet) {
  // create frame
  // stuffing
  // write frame
}

void setupLinkLayer() {
  link_layer.baud_rate = BAUDRATE;
  link_layer.sequence_num = 0;
  link_layer.timeout = 1;
  link_layer.num_transmissions = 3;
}

void assembleCtrlFrame(char addr, char ctrl, char** frame) {
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

enum state_t validateCtrlFrame(char addr,
                               char ctrl,
                               char* frame,
                               enum state_t curr_state) {
  switch (curr_state) {
    case START:
      if (frame[0] == FLAG_BYTE)
        return FLAG_RCV;
    case FLAG_RCV:
      if (frame[0] == ADDR_CE_RR)
        return A_RCV;
      else
        return START;
    case A_RCV:
      if (frame[0] == ctrl)
        return C_RCV;
      else if (frame[0] == FLAG_BYTE)
        return FLAG_RCV;
      else
        return START;
    case C_RCV:
      if (frame[0] == BCC1(addr, ctrl))
        return BCC_OK;
      else if (frame[0] == FLAG_BYTE)
        return FLAG_RCV;
      else
        return START;
    case BCC_OK:
      if (frame[0] == FLAG_BYTE)
        return STOP;
      else
        return START;
    default:
      return START;
  }
}

// TODO #6
int establishment() {
  /* start communication
    1. emitter sends SET ctrl frame
    2. emitter waits for UA ctrl frame that will be sent by receiver
    3. emitter validates UA ctrl frame and effectively starts communication
  */

  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  char set[5], response[1];

  assembleCtrlFrame(ADDR_CE_RR, CTRL_SET, set);

  while (msg_state != STOP && try < 4) {
    if (flag) {
      num_bytes_written = writeCtrlFrame(set);

      printf("Try #%d: %x %x %x %x %x (%d bytes)\n", try, set[0], set[1],
             set[2], set[3], set[4], num_bytes_written);

      alarm(3);
      msg_state = START;
    }

    flag = 0;

    num_bytes_read = readCtrlFrame(response);
    msg_state = validateCtrlFrame(ADDR_CE_RR, CTRL_UA, response, msg_state);

    if (msg_state == STOP) {
      printf("UA received.\n");
      return 0;
    } else {
      printf("UA not received.\n");
      return -1;
    }
  }
}

// TODO #7
int termination() {
  /* end communication
    1. emitter sends DISC ctrl frame
    2. emitter waits for DISC ctrl frame that will be sent back by receiver
    3. emitter sends UA ctrl frame and effectively ends communication
  */

  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  char disc[5], ua[5], response[1];

  assembleCtrlFrame(ADDR_CE_RR, CTRL_DISC, disc);

  while (msg_state != STOP && try < 4) {
    if (flag) {
      num_bytes_written = writeCtrlFrame(disc);

      printf("Try #%d: %x %x %x %x %x (%d bytes)\n", try, disc[0], disc[1],
             disc[2], disc[3], disc[4], num_bytes_written);

      alarm(3);
      msg_state = START;
    }

    flag = 0;

    num_bytes_read = readCtrlFrame(response);
    msg_state = validateCtrlFrame(ADDR_CE_RR, CTRL_DISC, response, msg_state);

    if (msg_state != STOP) {
      printf("Receiver did not send DISC frame back to emitter.\n");
      return -1;
    }

    assembleCtrlFrame(ADDR_CE_RR, CTRL_UA, ua);

    num_bytes_written = writeCtrlFrame(ua);

    printf("Sent final UA to receiver: %x %x %x %x %x (%d bytes)\n", ua[0],
           ua[1], ua[2], ua[3], ua[4], num_bytes_written);
  }
}

// TODO #8
int packetToFrame(char* packet, char* frame) {
  // frame = FLAG | campo de endereçamento | campo de controlo | BCC1 | DADOS
  // | BCC2 | FLAG; pacote = Campo de controlo (1- dados) | N numero de
  // sequencia modulo 255 | L2 | L1 | P1 | ... | PK;  K = 256* L2 + L1
  return 0;
}

// TODO #9
int stuffing(char* frame) {
  return 0;
}

// TODO #10
int destuffing(char* frame) {
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

/*
int acknowledgment() {
  enum state_t msg_state = START;

  int num_bytes_written, num_bytes_read;
  char set_cmd[1], ua_cmd[5];

  while (msg_state != STOP) {
    /* read SET frame sent by emitter
    num_bytes_read = readCtrlFrame(set_cmd);

    if (num_bytes_read > 0)
      printf("%x \n", set_cmd[0]);

    /* validate SET frame sent by emitter
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

  /* assemble UA frame
  ua_cmd[0] = FLAG_BYTE;
  ua_cmd[1] = ADDR_CR_RE;
  ua_cmd[2] = CTRL_UA;
  ua_cmd[3] = BCC1(ADDR_CR_RE, CTRL_UA);
  ua_cmd[4] = FLAG_BYTE;

  /* send UA frame to emitter
  num_bytes_written = writeCtrlFrame(ua_cmd);
  printf("Sent to emissor: %x %x %x %x %x (%d bytes)\n", ua_cmd[0], ua_cmd[1],
         ua_cmd[2], ua_cmd[3], ua_cmd[4], num_bytes_written);
}
*/
