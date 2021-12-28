#include <stdlib.h>
#include "../include/commands.h"


int sendCommand(int socketfd, char *cmd)
{

  size_t cmd_len = strlen(cmd);

  if (write(socketfd, cmd, cmd_len) != cmd_len) {
    fprintf(stderr, "error sendComand()");
    return -1;
    }

    return 0;
}

//TODO ERRORS
void readResponse(int socketfd)
{
  FILE* file = fdopen(socketfd, "r");

  if (file == NULL) {
    fprintf(stderr, "error opening file");
    exit(-1);
  }

  char* buf = NULL;
  size_t buf_len = 0;

  while (getline(&buf, &buf_len, file) >= 0) {
    printf("%s", buf);

    if (buf[3] == ' ') {
      // verify codes
      break;
    }
  }
    return 0;
}

//TODO  related to previous missing all
void updateIpPort(int socketfd, char *ip, int *port)
{
  FILE* file = fdopen(socketfd, "r");

  if (file == NULL) {
    fprintf(stderr, "error opening file");
    exit(-1);
  }

  char* buf = NULL;
  size_t buf_len = 0;

  getline(&buf, &buf_len, file);
  printf("%s", buf);

  char ip1[4], ip2[4], ip3[4], ip4[4], port1[4], port2[4];

  strtok(buf, "(");
  strcpy(ip1, strtok(NULL, ","));
  strcpy(ip2, strtok(NULL, ","));
  strcpy(ip3, strtok(NULL, ","));
  strcpy(ip4, strtok(NULL, ","));

  snprintf(ip,17, "%s.%s.%s.%s", ip1, ip2, ip3, ip4);

  strcpy(port1, strtok(NULL, ","));
  strcpy(port2, strtok(NULL, ")"));

  *port = atoi(port1) * 256 + atoi(port2);
  return 0;
}