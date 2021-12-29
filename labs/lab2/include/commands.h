#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <stdio.h>
#include <string.h>

/**
 * @brief 
 * 
 * @param socketfd 
 * @param cmd 
 * @return int 
 */
int sendCommand(int socketfd, char* cmd);

/**
 * @brief 
 * 
 * @param socketfd 
 */
void readResponse(int socketfd);

/**
 * @brief 
 * 
 * @param socketfd 
 * @param ip 
 * @param port 
 */
void updateIpPort(int socketfd, char* ip, int* port);

#endif