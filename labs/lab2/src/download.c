#include <stdio.h>
#include <stdlib.h>
#include "../include/tcp.h"
#include "../include/parser.h"

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s ftp://[user:password@]host/url\n", argv[1]);
    }

    Data* data = malloc(sizeof(Data));

    parse_url(data, argv[1]);

    //get ip with hostname
    get_ip(data);

    execute(data);
}