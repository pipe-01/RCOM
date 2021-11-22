#pragma once
#include "macros.h"


void setUpReceive(int fd);

/**
 * @brief 
 * 
 * @param fd serial port descriptor
 * @return file information
 */
fileInfo receiveControlPackage(int fd);

int checkControlPackage(unsigned char *controlPackage, int *size, fileInfo *fileinfo);

/**
 * @brief 
 * 
 * @param fd serial port descriptor
 */
void handleDisconnection(int fd);