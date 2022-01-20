#ifndef _TCP_H_
#define _TCP_H_

#include "../include/parser.h"

/**
 * @brief 
 *  the ip from host name
 * @param data
 */
void get_ip(Data *data);

/**
 * @brief
 * main function that executes the flow of the program
 * @param data
 */
void execute(Data data);

/**
 * @brief
 * establishes ftp connection using ip and port 
 *
 * @param ip
 * @param port
 * @param socketfd returned as parameter
 */
void connection(char *ip, int port, int *socketfd);

/**
 * @brief
 * sends user and pass commands and reads respective answers
 * @param socket_A
 * @param user
 * @param password
 */
void login(int socket_A, char *user, char *password);

/**
 * @brief
 * sends pasv command and reads respective answer
 * establishes new connection
 * @param socket_A
 * @return int
 */
int setPASV(int *socket_A);

/**
 * @brief
 * sends rtr command and reads respective answer
 * transfers file retrieved
 * @param socket_A
 * @param socket_B
 * @param data
 */
void download(int socket_A, int socket_B, Data data);

/**
 * @brief 
 * 
 * @param socket_B 
 * @param filename 
 */
void save(int socket_B, char* filename);

/**
 * @brief
 *
 * @param socketfd
 * closes previously open file descriptors
 * @param socket_A 
 * @param socket_B 
 */
void disconnect(int socket_A, int socket_B);

#endif