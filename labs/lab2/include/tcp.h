#ifndef _TCP_H_
#define _TCP_H_

#include "../include/parser.h"

/**
 * @brief Get the ip object
 * 
 * @param data 
 */
void get_ip(Data* data);

/**
 * @brief 
 * 
 * @param data 
 */
void execute(Data* data);

/**
 * @brief 
 * 
 * @param ip 
 * @param port 
 * @param socketfd 
 */
void connection(char *ip, int port, int *socketfd);

/**
 * @brief 
 * 
 * @param socketfd 
 * @param user 
 * @param password 
 */
void login(int socketfd, char *user, char *password);

/**
 * @brief 
 * 
 * @param socketfd 
 * @param data 
 */
void setPASV(int* socketfd, Data* data);

/**
 * @brief 
 * 
 * @param socketfd 
 * @param file 
 */
void download(int socketfd, char *file);

/**
 * @brief 
 * 
 * @param socketfd 
 */
void disconnect(int socketfd);
#endif