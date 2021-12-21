#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../headers/tcp.h"
#include "../headers/parser.h"



int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s ftp://[user:password@]host/url\n", argv[1]);
    }

    struct Data data;
    parse_url(&data, argv[1]);

    //get ip with hostname

    //transfer file

    //exit
}