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
  --
      - open file
                       -- Application Layer
      - create control packet & llwrite      -- Application Layer
      - set serial port                      -- Physical Layer
      - create i-frame & write               -- Physical Layer

      - loooooooooop
        - read part of file                  -- Application Layer
        - create packet                      -- Application Layer
        - send packet to physical layer      -- Application Layer
        - create i-frame                     -- Physical Layer
        - wait for answer & act accordingly  -- Physical Layer

      - resend control packet and llwrite    -- Application Layer
      - disconnect serial port               -- Physical Layer
      - close file                           -- Application Layer
  */

  openFile(argv[2]); /* ./emitter /dev/ttyS0 nome_ficheiro */
  setupSerialPort(argv[1]);
  setupAlarm();

  establishment();

  sleep(2);
  resetSerialPort();
  closeFile(argv[2]);

  return 0;
}
