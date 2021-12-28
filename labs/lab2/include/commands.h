#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <stdio.h>
#include <string.h>

/**
 * @brief 
 * 
 * @param socketfd 
 * @param cmd 
 */
int sendCommand(int socketfd, char *cmd);

/**
 * @brief 
 * 
 * @param socketfd 
 * @param cmd 
 */
void readResponse(int socketfd);

/**
 * @brief 
 * 
 * @param ip 
 * @param port 
 */
void updateIpPort(int socketfd, char *ip, int *port);

#endif