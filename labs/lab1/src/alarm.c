#include "../include/alarm.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int flag = 1, try = 1;

void setupAlarm() {
  (void)signal(SIGALRM, handler);
}

void handler() {
  try++;
  printf("triimm trriim\n");
  flag = 1;
}
