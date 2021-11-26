#pragma once
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "macros.h"
#include "appLayer.h"
#include "alarme.h"
#include "transmitter.h"
#include "receive.h"


#define TRANSMITTER 0
#define RECEIVER 1

typedef struct applicationLayers{
    int fileDescriptor;
    int status;
} applicationLayer;

/**
 * @brief 
 * 
 * @param port 
 * @param role 
 * @return int 
 */
int llopen(int port, int role);

/**
 * @brief 
 * 
 * @param port 
 * @param f 
 * @param fname 
 * @return int 
 */
int llwrite(int port, FILE *f, char *fname);

/**
 * @brief 
 * 
 * @param fd 
 * @return int 
 */
int llread(int fd);

/**
 * @brief 
 * 
 * @param fd 
 * @param status 
 * @return int 
 */
int llclose(int fd, int status);