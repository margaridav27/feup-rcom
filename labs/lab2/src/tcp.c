#include "../include/tcp.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/commands.h"

void get_ip(Data *data)
{
    struct hostent *h;
    char *ip;

    if ((h = gethostbyname(data->host)) == NULL)
    {
        fprintf(stderr, "ERROR get_ip(): Unable to get ip address for %s",
                data->host);
        exit(-1);
    }

    ip = inet_ntoa(*((struct in_addr *)h->h_addr_list[0]));

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)h->h_addr_list[0])));

  strcpy(data->ip, ip);
  strcpy(data->host, h->h_name);
}

void execute(Data data)
{
    int socket_A, socket_B;

    printf("- Term A\n");
    connection(data.ip, 21, &socket_A);
    s_read(socket_A);

    login(socket_A, data.user, data.password);

    socket_B = setPASV(&socket_A);

    download(socket_A, socket_B, data);

    close(socket_A);
    close(socket_B);
}

void connection(char *ip, int port, int *socket_fd)
{
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr =
        inet_addr(ip); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port =
        htons(port); /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((*socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(-1);
    }

    /*connect to the server*/
    if (connect(*socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0)
    {
        perror("connect()");
        exit(-1);
    }
    printf("telnet %s %d\n", ip, port);
}

void login(int socket_A, char *user, char *password)
{
    char *cmd = malloc(8 + strlen(user));

    sprintf(cmd, "user %s\n", user);
    sendCommand(socket_A, cmd);
    s_read(socket_A);

  char* ptr = realloc(cmd, 8 + strlen(password));

  if (ptr == NULL)
    return;

    sprintf(cmd, "pass %s\n", password);
    sendCommand(socket_A, cmd);
    s_read(socket_A);

    free(cmd);
}

int setPASV(int *socket_A)
{
    char cmd[6];
    strcpy(cmd, "pasv\n");

    sendCommand(*socket_A, cmd);

    char *ip = malloc(17);
    int port, socket_B;

    s_readPASV(*socket_A, ip, &port);

    printf("- Term B\n");
    connection(ip, port, &socket_B);

    return socket_B;
}

void download(int socket_A, int socket_B, Data data)
{
    printf("- Term A\n");
    char *cmd = malloc(8 + strlen(data.path));
    sprintf(cmd, "retr %s\n", data.path);

    sendCommand(socket_A, cmd);

    free(cmd);

    printf("Term B\n");
    save(socket_B, data.filename);

    printf("- Term A\n");
    s_read(socket_A);
}

void save(int socket_B, char *filename)
{
    int fd = open(filename, O_CREAT | O_WRONLY, 0777);

    if (fd == -1)
    {
        fprintf(stderr, "error\n");
    }

  size_t read_bytes;
  int BUF_SIZE = 1;
  char buf[BUF_SIZE];

  do {
    read_bytes = read(socket_B, buf, BUF_SIZE);
    if (read_bytes > 0) {
      write(fd, buf, read_bytes);
    }
  } while (read_bytes > 0);

    close(fd);
}

void disconnect(int socket_A, int socket_B)
{
    close(socket_A);
    close(socket_B);
}
