#include "../include/alarm.h"
#include "../include/physical_layer.h"
#include "../include/serial_port.h"

#include <stdio.h>

int main(int argc, char** argv) {
  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
  //read SET command sent from emitter

  //printf("Received from emitter: \n");

  while (msg_state != STOP) {
    res = read(fd, rcv, 1);
    if (res > 0)
      printf("%x \n", rcv[0]);

    switch (msg_state) {
      case START:
        if (rcv[0] == FLAG)
          msg_state = FLAG_RCV;
        break;
      case FLAG_RCV:
        if (rcv[0] == MSG_BUF)
          msg_state = A_RCV;
        else
          msg_state = START;
        break;
      case A_RCV:
        if (rcv[0] == C_SET)
          msg_state = C_RCV;
        else if (rcv[0] == FLAG)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case C_RCV:
        if (rcv[0] == BCC)
          msg_state = BCC_OK;
        else if (rcv[0] == FLAG)
          msg_state = FLAG_RCV;
        else
          msg_state = START;
        break;
      case BCC_OK:
        if (rcv[0] == FLAG)
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

  //assemble UA command
  ua[0] = FLAG;
  ua[1] = CMD_BUF;
  ua[2] = C_UA;
  ua[3] = BCC;
  ua[4] = FLAG;

  //send UA command to emissor
  res = write(fd, ua, 5);
  printf("Sent to emissor: %x %x %x %x %x\n", ua[0], ua[1], ua[2], ua[3],
         ua[4]);
  */

  openFile(argv[2]); /* ./emitter /dev/ttyS1 nome_ficheiro */
  setupSerialPort(argv[1]);
  setupAlarm();

  // establishment(); 
  //establishment do lado do receiver é diferente, como vamos fazer??

  sleep(2);
  resetSerialPort();
  closeFile(argv[2]);
}
