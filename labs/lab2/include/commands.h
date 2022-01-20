#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <stdio.h>
#include <string.h>

#define FAILURE_CODE_I '5'
#define UNACCEPTED_CODE_I '4'

/**
 * @brief 
 * writes string cmd to file descriptor socketfd
 * @param socketfd 
 * @param cmd 
 */
int sendCommand(int socketfd, char *cmd);

/**
 * @brief 
 * reads and prints to the stdout the strings read from the file descriptor socketfd
 * @param socketfd 
 */
void s_read(int socketfd);

/**
 * @brief 
 * reads the answer for the command pasv and prints it to the stdout
 * parses ip and calculates port
 * @param socketfd 
 * @param ip 
 * @param port 
 */
void s_readPASV(int socketfd, char* ip, int* port);

#endif