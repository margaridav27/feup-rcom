#include "../include/application_layer.h"
#include "../include/alarm.h"
#include "../include/application_layer_macros.h"
#include "utils.c"

#include <stdio.h>

int main(int argc, char** argv) {
  if (checkArgs(argc, argv) == -1)
    return -1;

  setupAlarm();

  setID(RECEIVER_ID);

  communicate(argv[1], argv[2]);

  return 0;
}
