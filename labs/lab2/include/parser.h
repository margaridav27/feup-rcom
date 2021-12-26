#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>
#include <string.h>

#define FTP "ftp://"

typedef struct Data
{
    char user[256];
    char password[256];
    char host[256];
    char url[256];
    char file[256];
    char ip[128];

} data;

/**
 * @brief parses the URL received by arguments into the data struct
 * 
 * @param data returns struct filled with correct data
 * @param url full string received in arguments
 */
void parse_url(data *data, char *url);

#endif