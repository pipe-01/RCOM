#pragma once

#include "macros.h"
#include <fcntl.h>
#include "alarme.h"

#define C_START 0x02
#define C_DATA 0x01
#define T1 0x00
#define T2 0x01
#define L1 0x04

extern int numRetry;
extern int alarmFlag;

/**
 * @brief Set the Up Transmitter object
 * 
 * @param fd 
 * @return int 
 */
int setUpTransmitter(int fd);

/**
 * @brief 
 * 
 * @param port 
 * @param buffer 
 * @param size 
 * @param seqN 
 */
void sendData(int port, unsigned char *buffer, int size, int seqN);

/**
 * @brief 
 * 
 * @param port 
 * @param fSize 
 * @param fName 
 * @param controlField 
 * @return int 
 */
int sendControl(int port, int fSize, unsigned char *fName, int controlField);

/**
 * @brief 
 * 
 * @param fd 
 * @param controlPackage 
 * @param size 
 * @param bcc2 
 * @param s 
 */
void sendControlPackage(int fd, unsigned char *controlPackage, int *size, unsigned char bcc2, int s);

/**
 * @brief 
 * 
 * @param buf 
 * @param size 
 * @param n 
 * @param l1 
 * @param l2 
 * @return unsigned char* 
 */
unsigned char *generateDataPackage(unsigned char *buf, int *size, int n, int l1, int l2);

/**
 * @brief 
 * 
 * @param fSize 
 * @param fName 
 * @param packageSize 
 * @param controlfield 
 * @return unsigned char* 
 */
unsigned char *generateControlPackage(int fSize, unsigned char *fName, int *packageSize, int controlfield);
