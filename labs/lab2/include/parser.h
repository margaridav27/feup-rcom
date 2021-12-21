#ifndef _PARSER_H_
#define _PARSER_H_

struct Data {
    char* user;
    char* password;
    char* host;
    char* url;
    char* file;
};
void parse_url(struct Data* data, char* url);

#endif