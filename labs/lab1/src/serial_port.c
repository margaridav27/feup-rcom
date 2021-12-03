#include "../include/serial_port.h"
#include "../include/link_layer.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

extern link_layer_t link_layer;
struct termios oldtio;

int setupSerialPort() {
  struct termios newtio;

  /* open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C */

  /* save current port settings */
  if (tcgetattr(link_layer.fd, &oldtio) == -1) {
    perror("tcgetattr");
    return -1;
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = link_layer.baud_rate | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;     /* set input mode */
  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until x chars received */

  tcflush(link_layer.fd, TCIOFLUSH);

  if (tcsetattr(link_layer.fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    return -1;
  }

  printf("New termios structure set.\n\n");
  return 0;
}

int resetSerialPort() {
  if (tcsetattr(link_layer.fd, TCSANOW, &oldtio) == -1) {
    perror("tcsetattr");
    return -1;
  }
  return 0;
}