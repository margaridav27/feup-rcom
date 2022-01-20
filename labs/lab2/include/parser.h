#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>
#include <string.h>

#define FTP "ftp:"

typedef struct 
{
    char user[256];
    char password[256];
    char host[256];
    char path[256];
    char filename[256];
    char ip[128];

} Data;

/**
 * @brief parses the URL received by arguments into the data struct
 * 
 * @param data returns struct filled with correct data
 * @param url full string received in arguments
 */
void parse_url(Data* data, char* url);
void getfilename(Data* data);

#endif