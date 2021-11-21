#pragma once

#include "macros.h"
#include <fcntl.h>
#include "alarme.h"

#define TIMEOUT 5

extern int numRetry;
extern int alarmFlag;

int setUpTransmitter(int fd);
void sendData(int port, unsigned char *buffer, int size, int seqN);
int sendControl(int port, int fSize, unsigned char *fName, int controlField);
void sendControlPackage(int port, unsigned char *controlPackage, int *size, unsigned char bcc2, int s);
unsigned char *generateDataPackage(unsigned char *buf, int *size, int n, int l1, int l2);
unsigned char *generateControlPackage(int fSize, unsigned char *fName, int *packageSize, int controlfield);