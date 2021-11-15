#include "../include/physical_layer.h"

#define BIT(n) (0x01 << (n))

#define FLAG 0x0d

#define ADDR_CE_RR                                       \
  0x03 /* commands sent by emitter and responses sent by \
          receiver */
#define ADDR_CR_RE                                        \
  0x01 /* commands sent by receiver and responses sent by \
          emitter */

#define CTRL_SET 0x03  /* setup */
#define CTRL_DISC 0x0b /* disconnect */
#define CTRL_UA 0x07   /* unnumbered acknowledgment */
#define CTRL_RR(r) \
  0x05 | (((r) % 2) ? BIT(7) : 0x00) /* receiver ready - positive ACK */
#define CTRL_REJ(r) \
  0x01 | (((r) % 2) ? BIT(7) : 0x00) /* receiver reject - negative ACK */

#define BCC1(addr, ctrl) ((addr) ^ (ctrl)) /* protection field */

#define BAUDRATE 0xB38400

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
  frame[0] = FLAG;
  frame[1] = ADDR_CR_RE;
  frame[2] = ctrl;
  frame[3] = BCC1(addr, ctrl);
  frame[4] = FLAG;
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
        if (response[0] == FLAG)
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
        else if (response[0] == FLAG)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case C_RCV:
        if (response[0] == BCC1(ADDR_CE_RR, CTRL_UA))
          msg_state = BCC_OK;
        else if (response[0] == FLAG)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case BCC_OK:
        if (response[0] == FLAG)
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
