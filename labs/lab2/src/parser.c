#include "../include/parser.h"
#include <stdlib.h>

void parse_url(data *data, char *url)
{
    char *ftp = strtok(url, "/");
    char *url = strtok(url, "/");
    strcpy(data->file, strtok(NULL, ""));

    if (!strcmp(ftp, FTP))
    {
        fprintf(stderr, "invalid url ftp://\n");
        exit(-1);
    }

    strcpy(data->user, strtok(url, ":"));

    strcpy(data->password, strtok(NULL, "@"));

    if (data->password == NULL)
    {
        strcpy(data->user, "anonymous");
        strcpy(data->password, "something");
    }

    printf("rest url: % s", url);
    strcpy(data->host, url);
}