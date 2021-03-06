#include "../include/link_layer.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/link_layer_macros.h"
#include "../include/serial_port.h"

enum state_t { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP, ERROR };

link_layer_t link_layer;
extern int flag, try;

int llopen(char* port, flag_t flag) {
  link_layer.status = flag;

  link_layer.fd = open(port, O_RDWR | O_NOCTTY);
  if (link_layer.fd < 0) {
    perror("llopen");
    return -1;
  }

  /* setup serial port */
  setupLinkLayer();
  setupSerialPort(link_layer.fd);

  /* establish connection by sending SET ctrl frame */
  if (flag == TRANSMITER)
    establishmentTransmitter();
  else
    establishmentReceiver();

  return 0;
}

int llclose() {
  if (link_layer.status == TRANSMITER)
    terminationTransmitter();
  else
    terminationReceiver();

  resetSerialPort();
  close(link_layer.fd);

  return 0;
}

unsigned char getBCC2(unsigned char* data, int sz) {
  unsigned char BCC2 = data[0];
  for (int i = 1; i < sz; i++)
    BCC2 ^= data[i];
  return BCC2;
}

int packetToFrame(unsigned char* packet, unsigned char* frame, int packet_sz) {
  frame[0] = FLAG_BYTE;
  frame[1] = ADDR_CE_RR;
  frame[2] = link_layer.sequence_num;
  frame[3] = BCC1(frame[1], frame[2]);

  memcpy(frame + 4, packet, packet_sz);

  frame[4 + packet_sz] = getBCC2(packet, packet_sz);
  frame[5 + packet_sz] = FLAG_BYTE;

  return 0;
}

void stuffing(unsigned char* frame, int* frame_sz) {
  unsigned char* stuffed_frame = malloc(2 * *frame_sz);
  int stuffed_frame_ix = 0;
  unsigned char bcc = frame[*frame_sz - 2];

  for (int i = 4; i < *frame_sz - 2; i++) {
    if (frame[i] == FLAG_BYTE) {
      stuffed_frame[stuffed_frame_ix] = ESCAPE_BYTE;
      stuffed_frame_ix++;
      stuffed_frame[stuffed_frame_ix] = FLAG_STUFFING_BYTE;
    } else if (frame[i] == ESCAPE_BYTE) {
      stuffed_frame[stuffed_frame_ix] = frame[i];
      stuffed_frame_ix++;
      stuffed_frame[stuffed_frame_ix] = ESCAPE_STUFFING_BYTE;
    } else {
      stuffed_frame[stuffed_frame_ix] = frame[i];
    }
    stuffed_frame_ix++;
  }

  memcpy(frame + 4, stuffed_frame, stuffed_frame_ix);

  frame[4 + stuffed_frame_ix] = bcc;
  frame[5 + stuffed_frame_ix] = FLAG_BYTE;

  *frame_sz = stuffed_frame_ix + 6;
}

void destuffing(unsigned char* frame, int* frame_sz) {
  char* destuffed_frame = malloc(*frame_sz);
  int destuffed_frame_ix = 0, frame_ix = 0;

  destuffed_frame[destuffed_frame_ix] = ESCAPE_BYTE;
  while (frame_ix < *frame_sz) {
    if ((frame[frame_ix] == ESCAPE_BYTE) &&
        (frame[frame_ix + 1] == ESCAPE_STUFFING_BYTE)) {
      destuffed_frame[destuffed_frame_ix] = ESCAPE_BYTE;
      frame_ix++;
    } else if ((frame[frame_ix] == ESCAPE_BYTE) &&
               (frame[frame_ix + 1] == FLAG_STUFFING_BYTE)) {
      destuffed_frame[destuffed_frame_ix] = FLAG_BYTE;
      frame_ix++;
    } else {
      destuffed_frame[destuffed_frame_ix] = frame[frame_ix];
    }
    frame_ix++;
    destuffed_frame_ix++;
  }

  *frame_sz = destuffed_frame_ix;
  memcpy(frame, destuffed_frame, destuffed_frame_ix);
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

enum state_t validateIFrame(unsigned char addr,
                            unsigned char* frame,
                            enum state_t curr_state) {
  enum state_t next_state = START;

  switch (curr_state) {
    case START:
      if (frame[0] == FLAG_BYTE)
        next_state = FLAG_RCV;
      else
        next_state = ERROR;
      break;
    case FLAG_RCV:
      if (frame[0] == addr)
        next_state = A_RCV;
      else
        next_state = ERROR;
      break;
    case A_RCV:
      if (frame[0] == (BIT(7) & link_layer.sequence_num)) {
        next_state = C_RCV;
      } else
        next_state = ERROR;
      break;
    case C_RCV:
      if (frame[0] == BCC1(addr, link_layer.sequence_num))
        next_state = BCC_OK;
      else
        next_state = ERROR;
      break;
    default:
      next_state = ERROR;
      break;
  }

  return next_state;
}

int llwrite(unsigned char* packet, int packet_sz) {
  int frame_sz = 6 + packet_sz;
  unsigned char* frame = malloc(2 * frame_sz);

  packetToFrame(packet, frame, packet_sz);

  stuffing(frame, &frame_sz);

  int num_bytes_read = 0;
  unsigned char res[5];

  for (;;) {
    writeFrame(frame, frame_sz);

    alarm(1);

    printf("[llwrite] I-frame sent to receiver.\n\n");

    while (try < 5 && num_bytes_read < 5) num_bytes_read = readFrame(res, 5);
    if (num_bytes_read == 5) break;
  }

  link_layer.sequence_num = ((~link_layer.sequence_num) & BIT(7));
  unsigned char RR = CTRL_RR(link_layer.sequence_num);

  if (res[2] == RR) return 0;

  link_layer.sequence_num = ((~link_layer.sequence_num) & BIT(7));
  return -1;
}

unsigned char* llread() {
  unsigned char* buffer = NULL;
  enum state_t msg_state = START;

  int num_bytes_read, ix = 0;

  unsigned char i_frame_ix[1], i_frame_header[5], ctrl_frame[5];

  /* read and check header validity */
  while (msg_state != BCC_OK && msg_state != ERROR) {
    num_bytes_read = readFrame(i_frame_ix, 1);

    if (num_bytes_read > 0) {
      i_frame_header[ix++] = i_frame_ix[0];
      msg_state = validateIFrame(ADDR_CE_RR, i_frame_ix, msg_state);
    }
  }

  /* read an check data validity */
  unsigned char i_frame_data[200];
  int data_ix = 0;

  for (;;) {
    readFrame(i_frame_ix, 1);
    if (i_frame_ix[0] == FLAG_BYTE) break;
    i_frame_data[data_ix++] = i_frame_ix[0];
  }

  int frame_data_sz = data_ix - 1; 
  unsigned char bcc2_before_destuffing = i_frame_data[frame_data_sz];

  if (msg_state != BCC_OK) { /* errors in header */
    printf("[llread] I-frame with errors in header received from transmitter.\n\n");

    assembleCtrlFrame(ADDR_CR_RE, CTRL_REJ(link_layer.sequence_num), ctrl_frame);

    printf("[llread] REJ sent to transmitter.\n\n");
  } else { /* header with no errors */
    destuffing(i_frame_data, &frame_data_sz);

    unsigned char bcc2_after_destuffing = getBCC2(i_frame_data, frame_data_sz);

    if (bcc2_after_destuffing != bcc2_before_destuffing) { 
      if (i_frame_header[2] != link_layer.sequence_num) { /* repeated frame */
        printf("[llread] I-frame already received from transmitter.\n\n");

        assembleCtrlFrame(ADDR_CR_RE, CTRL_RR(link_layer.sequence_num), ctrl_frame);

        printf("[llread] RR sent to transmitter.\n\n");
      } else { /* first time receiving frame */
        printf("[llread] I-frame with errors in data received from transmitter.\n\n");

        assembleCtrlFrame(ADDR_CR_RE, CTRL_REJ(link_layer.sequence_num), ctrl_frame);

        for (;;) { /* to discard the frame on needs to read it first */
          readFrame(i_frame_ix, 1);
          if (i_frame_ix[0] == FLAG_BYTE) break;
        }

        sleep(4);
        printf("[llread] REJ sent to transmitter.\n\n");
      }
    } else { /* frame with no errors */
      printf("[llread] I-frame received from transmitter.\n\n");

      buffer = malloc(frame_data_sz);
      memcpy(buffer, i_frame_data, frame_data_sz);
      link_layer.sequence_num = ((~link_layer.sequence_num) & BIT(7));
      assembleCtrlFrame(ADDR_CR_RE, CTRL_RR(link_layer.sequence_num), ctrl_frame);

      printf("[llread] RR sent to transmitter.\n\n");
    }
  }

  writeFrame(ctrl_frame, 5);
  return buffer;
}

void setupLinkLayer() {
  link_layer.baud_rate = BAUDRATE;
  link_layer.sequence_num = 0x00;
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

int writeFrame(unsigned char* frame, int sz) {
  return write(link_layer.fd, frame, sz);
}

int readFrame(unsigned char* frame, int sz) {
  return read(link_layer.fd, frame, sz);
}

int establishmentTransmitter() {
  try = 1;

  enum state_t msg_state = START;

  int num_bytes_read;
  unsigned char set[5], response[1];

  assembleCtrlFrame(ADDR_CE_RR, CTRL_SET, set);

  while (msg_state != STOP && try < 4) {
    if (flag && msg_state != STOP) {
      writeFrame(set, sizeof(set));
      printf("[establishmentTransmitter] SET supervision frame sent to receiver: %x %x %x %x %x\n\n", 
             set[0], set[1], set[2], set[3], set[4]);

      alarm(6);
      msg_state = START;
    }

    flag = 0;

    num_bytes_read = readFrame(response, 1);
    if (num_bytes_read > 0) msg_state = validateCtrlFrame(ADDR_CR_RE, CTRL_UA, response, msg_state);
  }

  if (msg_state == STOP) {
    printf("[establishmentTransmitter] UA supervision frame received from transmitter.\n\n");
    return 0;
  } else {
    printf("[establishmentTransmitter] UA supervision frame not received from transmitter.\n\n");
    return -1;
  }
}

int establishmentReceiver() {
  enum state_t msg_state = START;

  int num_bytes_read;
  unsigned char set_cmd[1], ua_cmd[5];

  while (msg_state != STOP) {
    num_bytes_read = readFrame(set_cmd, 1);
    if (num_bytes_read > 0) msg_state = validateCtrlFrame(ADDR_CE_RR, CTRL_SET, set_cmd, msg_state);
  }

  if (msg_state != STOP) {
    printf("[establishmentReceiver] SET supervision frame not received from transmitter.\n\n");
    return -1;
  } else {
    printf("[establishmentReceiver] SET supervision frame received from transmitter.\n\n");
  }

  assembleCtrlFrame(ADDR_CR_RE, CTRL_UA, ua_cmd);

  writeFrame(ua_cmd, sizeof(ua_cmd));
  printf("[establishmentReceiver] UA supervision frame sent to transmitter: %x %x %x %x %x\n\n",
         ua_cmd[0], ua_cmd[1], ua_cmd[2], ua_cmd[3], ua_cmd[4]);

  return 0;
}

int terminationTransmitter() {
  try = 1;
  enum state_t msg_state = START;

  int num_bytes_read;
  unsigned char disc[5], ua[5], response[1];

  assembleCtrlFrame(ADDR_CE_RR, CTRL_DISC, disc);

  while (msg_state != STOP && try < 4) {
    if (flag) {
      writeFrame(disc, sizeof(disc));
      printf("[terminationTransmitter] DISC supervision frame sent to receiver: %x %x %x %x %x\n\n",
             disc[0], disc[1], disc[2], disc[3], disc[4]);

      alarm(3);
      msg_state = START;
    }

    flag = 0;

    num_bytes_read = readFrame(response, 1);
    if (num_bytes_read > 0) msg_state = validateCtrlFrame(ADDR_CR_RE, CTRL_DISC, response, msg_state);
  }

  if (msg_state != STOP) {
    printf("[terminationTransmitter] DISC supervision frame not sent back to transmitter.\n\n");
    return -1;
  } else {
    printf("[terminationTransmitter] DISC supervision frame sent back to transmitter.\n\n");
  }

  assembleCtrlFrame(ADDR_CE_RR, CTRL_UA, ua);

  writeFrame(ua, sizeof(ua));
  printf("[terminationTransmitter] Final UA supervision frame sent to transmitter: %x %x %x %x %x\n\n",
         ua[0], ua[1], ua[2], ua[3], ua[4]);

  return 0;
}

int terminationReceiver() {
  enum state_t msg_state = START;

  int num_bytes_read;
  unsigned char receive_cmd[1], disc_cmd[5];

  while (msg_state != STOP) {
    num_bytes_read = readFrame(receive_cmd, 1);
    if (num_bytes_read > 0) msg_state = validateCtrlFrame(ADDR_CE_RR, CTRL_DISC, receive_cmd, msg_state);
  }

  if (msg_state != STOP) {
    printf("[terminationReceiver] DISC supervision frame not sent from transmitter.\n\n");
    return -1;
  } else {
    printf("[terminationReceiver] DISC supervision frame sent from transmitter.\n\n");
  }

  assembleCtrlFrame(ADDR_CR_RE, CTRL_DISC, disc_cmd);

  writeFrame(disc_cmd, sizeof(disc_cmd));
  printf("[terminationReceiver] DISC supervision frame sent back to transmitter: %x %x %x %x %x\n\n",
         disc_cmd[0], disc_cmd[1], disc_cmd[2], disc_cmd[3], disc_cmd[4]);

  msg_state = START;

  while (msg_state != STOP) {
    num_bytes_read = readFrame(receive_cmd, 1);
    if (num_bytes_read > 0) msg_state = validateCtrlFrame(ADDR_CE_RR, CTRL_UA, receive_cmd, msg_state);
  }

  if (msg_state != STOP) {
    printf("[terminationReceiver] Final UA supervision frame not received from transmitter.\n\n");
    return -1;
  } else {
    printf("[terminationReceiver] Final UA supervision frame received from transmitter.\n");
  }
  return 0;
}
