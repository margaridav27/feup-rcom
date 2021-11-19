#include "../include/application_layer.h"
#include "../include/alarm.h"
#include "../include/application_layer_macros.h"
#include "utils.c"

#include <stdio.h>

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

  setID(RECEIVER_ID);

  communicate(argv[1], "");

  return 0;
}
