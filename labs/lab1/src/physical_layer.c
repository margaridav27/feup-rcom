#include "../include/physical_layer.h"

#define BIT(n) (0x01 << (n))

#define MSG_FLAG 0x0d

#define MSG_ADDR_CE_RR                                                 \
  0x03 /* commands sent by emitter and num_bytes_writtenponses sent by \
          receiver */
#define MSG_ADDR_CR_RE                                                  \
  0x01 /* commands sent by receiver and num_bytes_writtenponses sent by \
          emitter */

#define MSG_CTRL_SET 0x03  /* setup */
#define MSG_CTRL_DISC 0x0b /* disconnect */
#define MSG_CTRL_UA 0x07   /* unnumbered acknowledgment */
#define MSG_CTRL_RR(r) \
  0x05 | (((r) % 2) ? BIT(7) : 0x00) /* receiver ready - positive ACK */
#define MSG_CTRL_REJ(r) \
  0x01 | (((r) % 2) ? BIT(7) : 0x00) /* receiver reject - negative ACK */
#define MSG_BCC1(addr, ctrl) ((addr) ^ (ctrl)) /* protection field */

#define BAUDRATE B38400

enum state_t { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP };

struct link_layer_t link_layer;
extern int flag, try;

void setupLinkLayer() {
  link_layer.baud_rate = BAUDRATE;
  link_layer.sequence_num = 0;
  link_layer.timeout = 1;
  link_layer.num_transmissions = 3;
}

int establishment() {
  enum state_t msg_state = START;

  int num_bytes_written;
  unsigned char set_cmd[5];
  unsigned char response[1];

  createFrame(MSG_CTRL_SET, set_cmd);

  while (msg_state != STOP && try < 4) {
    if (flag) {
      /* send SET command to receiver */
      num_bytes_written = write(link_layer.port, set_cmd, 5);
      printf("try #%d: %x %x %x %x %x (%d bytes)\n", try, set_cmd[0],
             set_cmd[1], set_cmd[2], set_cmd[3], set_cmd[4], num_bytes_written);

      alarm(3);
      msg_state = START;
    }

    flag = 0;
    /* read UA command sent by receiver */
    num_bytes_written = read(link_layer.port, response, 1);

    /* validate UA command sent by receiver */
    switch (msg_state) {
      case START:
        if (response[0] == MSG_FLAG)
          msg_state = FLAG_RCV;
        break;
      case FLAG_RCV:
        if (response[0] == MSG_ADDR_CR_RE)
          msg_state = A_RCV;
        else
          msg_state = START;
        break;
      case A_RCV:
        if (response[0] == MSG_CTRL_UA)
          msg_state = C_RCV;
        else if (response[0] == MSG_FLAG)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case C_RCV:
        if (response[0] == MSG_BCC1(MSG_CTRL_UA, MSG_ADDR_CR_RE))
          msg_state = BCC_OK;
        else if (response[0] == MSG_FLAG)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case BCC_OK:
        if (response[0] == MSG_FLAG)
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

int createCommandFrame(unsigned char control, char** cmd) {
  cmd[0] = MSG_FLAG;
  cmd[1] = MSG_ADDR_CR_RE;
  cmd[2] = control;
  cmd[3] = MSG_BCC1(control, MSG_ADDR_CE_RR);
  cmd[4] = MSG_FLAG;
}