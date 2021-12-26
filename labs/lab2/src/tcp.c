#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/commands.h"

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

void get_ip(data *data)
{
    struct hostent *h;
    char *ip;

    /**
 * The struct hostent (host entry) with its terms documented

    struct hostent {
        char *h_name;    // Official name of the host.
        char **h_aliases;    // A NULL-terminated array of alternate names for the host.
        int h_addrtype;    // The type of address being returned; usually AF_INET.
        int h_length;    // The length of the address in bytes.
        char **h_addr_list;    // A zero-terminated array of network addresses for the host.
        // Host addresses are in Network Byte Order.
    };

    #define h_addr h_addr_list[0]	The first address in h_addr_list.
*/
    if ((h = gethostbyname(data->host)) == NULL)
    {
        fprintf(stderr, "ERROR get_ip(): Unable to get ip address for %s", host);
        exit(-1);
    }

    *ip = inet_ntoa(*((struct in_addr *)h->h_addr));
    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)h->h_addr)));

    strcpy(data->ip, ip);
    return 0;
}

void downloadFile(data *data)
{

    int socketfd;
    connect(data->ip, 21, &socketfd);

    login(socketfd, data->user, data->password);

    setPASV(&socketfd, &data);

    download(socketfd);

    disconnect(socketfd);
}

void connect(char *ip, int port, int *socketfd)
{
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(SERVER_PORT);            /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(*sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect()");
        exit(-1);
    }
}

void login(int socketfd, char *user, char *password)
{
    char *cmd = malloc(6 + strlen(user));

    sprintf(cmd, "user %s\n", user);
    sendCommand(socketfd, cmd);
    readResponse();

    realloc(cmd, 5 + strlen(password));

    sendCommand(socketfd, cmd);
    readResponse();

    free(cmd);
}

void setPASV(int *socketfd, data *data)
{
    char cmd[6];
    strcpy(cmd, "pasv\n");

    sendCommand(socketfd, cmd);

    char *ip;
    int port;
    updateIpPort(ip, &port);

    connect(ip, port, socketfd);
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
