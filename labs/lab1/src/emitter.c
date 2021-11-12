/*Non-Canonical Input Processing*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define CMD_BUF 0x03
#define MSG_BUF 0x01
#define FLAG 0x7d
#define SET 0x03
#define UA 0x07

enum state { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP };

int flag = 1, try = 1;

void handler() {
  printf("alarme # %d\n", try);
  flag = 1;
  try++;
}

int main(int argc, char** argv) {
  unsigned char cmd[5], ans[1], ack[5];
  enum state msg_state = START;
  int fd, c, res;
  struct termios oldtio, newtio;
  int i, sum = 0, speed = 0;

  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
  open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    perror(argv[1]);
    exit(-1);
  }

  /* save current port settings */
  if (tcgetattr(fd, &oldtio) == -1) {
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;     /* set input mode */
  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;  /* blocking read until x chars received */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  printf("New termios structure set\n");

  /* assemble SET command */
  cmd[0] = FLAG;
  cmd[1] = CMD_BUF;
  cmd[2] = SET;
  cmd[3] = CMD_BUF ^ SET;
  cmd[4] = FLAG;

  printf("Feedback from receiver: \n");

  (void)signal(SIGALRM, handler);

  /* read UA command sent from receiver */
  while (msg_state != STOP && try < 4) {
    if (flag) {
      /* send SET command to receiver */
      printf("1\n");
      res = write(fd, cmd, 5);
      alarm(3);
      msg_state = START;
      printf("Sent to receiver: %x %x %x %x %x (%d bytes)\n", cmd[0], cmd[1],
             cmd[2], cmd[3], cmd[4], res);
    }

    flag = 0;
    res = read(fd, ans, 1);

    switch (msg_state) {
      case START:
        if (ans[0] == FLAG)
          msg_state = FLAG_RCV;
        break;
      case FLAG_RCV:
        if (ans[0] == MSG_BUF)
          msg_state = A_RCV;
        else
          msg_state = START;
        break;
      case A_RCV:
        if (ans[0] == UA)
          msg_state = C_RCV;
        else if (ans[0] == FLAG)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case C_RCV:
        if (ans[0] == MSG_BUF ^ UA)
          msg_state = BCC_OK;
        else if (ans[0] == FLAG)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case BCC_OK:
        if (ans[0] == FLAG)
          msg_state = STOP;
        else
          msg_state = START;
        break;
      default:
        msg_state = START;
        break;
    }
  }

  if (msg_state == STOP)
    printf("\nUA received\n");
  else
    printf("\nUA not received\n");

  sleep(2);
  if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
