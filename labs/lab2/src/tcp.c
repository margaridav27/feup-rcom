#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "../include/commands.h"
#include "../include/tcp.h"


void get_ip(Data* data) {
  struct hostent* h;
  char* ip;

  if ((h = gethostbyname(data->host)) == NULL) {
    fprintf(stderr, "ERROR get_ip(): Unable to get ip address for %s",
            data->host);
    exit(-1);
  }

  ip = inet_ntoa(*((struct in_addr*)h->h_addr_list[0]));

  printf("Host name  : %s\n", h->h_name);
  printf("IP Address : %s\n", inet_ntoa(*((struct in_addr*)h->h_addr_list[0])));


  strcpy(data->ip, ip);
  strcpy(data->host, h->h_name);
  return 0;
}

void execute(Data* data) {
  int socketfd;

  connection(data->ip, 21, &socketfd);

  login(socketfd, data->user, data->password);

  setPASV(&socketfd, &data);

  // download(socketfd);

  //disconnect(socketfd);
}

void connection(char *ip, int port, int *socketfd)
{
  struct sockaddr_in server_addr;
  /*server address handling*/
  bzero((char*)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(port); /*server TCP port must be network byte ordered */

  /*open a TCP socket*/
  if ((*socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    exit(-1);
    }
    /*connect to the server*/
    if (connect(*socketfd, (struct sockaddr*)&server_addr,
                sizeof(server_addr)) < 0) {
      perror("connect()");
      exit(-1);
    }
    readResponse(*socketfd);
    
}

void login(int socketfd, char *user, char *password)
{
  char* cmd = malloc(8 + strlen(user));

  sprintf(cmd, "user %s\n", user);
  sendCommand(socketfd, cmd);
  readResponse(socketfd);

  realloc(cmd, 8 + strlen(password));
  sprintf(cmd, "pass %s\n", password);

  sendCommand(socketfd, cmd);
  readResponse(socketfd);

  free(cmd);
}

void setPASV(int* socketfd, Data* data) {
  char cmd[6];
  strcpy(cmd, "pasv\n");

  sendCommand(*socketfd, cmd);

  char* ip = malloc(17);
  int port;
  updateIpPort(*socketfd, ip, &port);
  close(*socketfd);
  connection(ip, port, socketfd);
}

void download(int socketfd, char *file)
{
    int fd = open(file, O_CREAT | O_WRONLY, "0777");

    if (fd == -1)
    {
        fprintf(stderr, "error\n");
    }

    size_t read_bytes, written_bytes;
    int BUF_SIZE = 20;
    char buf[BUF_SIZE];

    while ((read_bytes = read(socketfd, buf, BUF_SIZE)) > 0)
    {
        written_bytes = write(fd, buf, read_bytes);

        if (read_bytes > written_bytes)
        {
            close(fd);
            fprintf(stderr, "failed");
            return -1;
        }
    }

    close(fd);
    return 0;
}

void disconnect(int socketfd)
{
    close(socketfd);
}
