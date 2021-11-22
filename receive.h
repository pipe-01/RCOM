#pragma once
#include "macros.h"

/**
 * @brief Set the Up Receive object
 * 
 * @param fd 
 */
void setUpReceive(int fd);

/**
 * @brief 
 * 
 * @param fd serial port descriptor
 * @return file information
 */
fileInfo receiveControlPackage(int fd);

/**
 * @brief 
 * 
 * @param controlPackage 
 * @param size 
 * @param fileinfo 
 * @return int 
 */
int checkControlPackage(unsigned char *controlPackage, int *size, fileInfo *fileinfo);

/**
 * @brief 
 * 
 * @param fd serial port descriptor
 */
void handleDisconnection(int fd);