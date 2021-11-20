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