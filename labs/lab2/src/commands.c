#include "../include/commands.h"

int sendCommand(int socketfd, char *cmd)
{
    size_t cmd_len = strlen(cmd);

    if (write(socketfd, cmd, strlen(cmd)) != cmd_len)
    {
        fprintf(stderr, "error sendComand()");
        return -1;
    }
    return 0;
}

//TODO  missing a lot
void readResponse(int socketfd, char *cmd)
{
    char *buf;
    size_t buf_len = 0;

    while (1)
    {
        getline(buf, buf_len, socketfd);
        printf("%s\n", buf);
    }
    return 0;
}

//TODO  related to previous missing all
void updateIpPort(char *ip, int *port)
{
}