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


int llopen(int port, int role);
int llwrite(int port, FILE *f, char *fname);
int llread(int fd, char * buffer);
int llclose(int fd);