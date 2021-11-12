#include "../include/alarm.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int flag = 1, try = 1;

void setupAlarm() {
  (void)signal(SIGALRM, handler);
}

void handler() {
  printf("alarme # %d\n", try);
  flag = 1;
  try++;
}
