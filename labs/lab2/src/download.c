#include <stdio.h>
#include <stdlib.h>
#include "../include/parser.h"
#include "../include/tcp.h"

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s ftp://[user:password@]host/url\n", argv[1]);
  }

  Data *data = malloc(sizeof(Data));

  parse_url(data, argv[1]);

  // get ip by hostname
  get_ip(data);

  execute(*data);
}