#include "../include/alarm.h"
#include "../include/application_layer.h"
#include "../include/application_layer_macros.h"
#include "utils.c"

#include <stdio.h>

int checkArgs(int argc, char** argv) {
  if ((argc < 3) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    return -1;
  }
  if (access(argv[2], R_OK) < 0) {
    printf("Usage:\tnserial file doesn't exist");
    return -1;
  }
  return 0;
}

int main(int argc, char** argv) {
  if (checkArgs(argc, argv) == -1)
    return -1;

  setupAlarm();

  setID(TRANSMITTER_ID);

  communicate(argv[1], argv[2]);

  return 0;
};