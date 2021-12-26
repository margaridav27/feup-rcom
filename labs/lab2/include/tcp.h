#ifndef _TCP_H_
#define _TCP_H_

/**
 * @brief Get the ip object
 * 
 * @param data 
 */
void get_ip(data *data);

/**
 * @brief 
 * 
 * @param data 
 */
void downloadFile(data *data);

/**
 * @brief 
 * 
 * @param ip 
 * @param port 
 * @param socketfd 
 */
void connect(char *ip, int port, int *socketfd);

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
void setPASV(int *socketfd, data *data);

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