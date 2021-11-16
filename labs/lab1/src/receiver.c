#include "../include/alarm.h"
#include "../include/application_layer.h"

#include <stdio.h>

// TODO #4
int checkArgs(int argc, char** argv) {
  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;
  }

  return 0;
}

int main(int argc, char** argv) {
  if (checkArgs(argc, argv) == -1)
    return -1;

  setupAlarm();

  emmit(argv[1], argv[2]);

  return 0;
}
