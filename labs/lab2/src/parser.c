#include "../include/parser.h"
#include <stdlib.h>

void parse_url(Data* data, char* url) {
  char* ftp = strtok(url, "/");
  char *user_pass_host = strtok(NULL, "/");
  
  strcpy(data->file, strtok(NULL, ""));

  if (strcmp(ftp, FTP) != 0) {
    fprintf(stderr, "invalid url ftp://\n");
    exit(-1);
    }

    strcpy(data->user, strtok(user_pass_host, ":"));
    char* pass = strtok(NULL, "@");

    if (pass == NULL)
    {
      char user[10] = "anonymous";
      char password[10] = "something";

      strcpy(data->user, user);
      strcpy(data->password, password);
      strcpy(data->host, user_pass_host);

    } else {
      strcpy(data->password, pass);
      strcpy(data->host, strtok(NULL, ""));
    }
}