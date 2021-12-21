#ifndef _TCP_H_
#define _TCP_H_

void get_ip(char *host, char **ip);

void transfer(struct Data data, char** ip);

void client ();

#endif