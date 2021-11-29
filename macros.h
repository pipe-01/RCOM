#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "alarme.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS5"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define I 1
#define S 0

#define MAX_RETRY 5
#define TIMEOUT 3
#define MAX_SIZE 1024

//Stuffing macros
#define ESCAPEMENT 0x7D
#define REPLACE_FLAG 0x5E
#define REPLACE_ESCAPEMENT 0x5D

#define FLAG 0x7E
#define A_TRM 0x03
#define A_REC 0x01
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_R0 0x05
#define C_R1 0x85
#define C_REJ0 0x81
#define C_REJ1 0x01
#define BCC(X, Y) (X) ^ (Y)


typedef struct{
    unsigned int numRetry;
    unsigned int alarmFlag;
} AlarmData;

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} State_Machine;

typedef struct
{
    unsigned char *filename;
    int size;
} fileInfo;


/**
 * @brief Sends message to state machine
 * 
 * @param fd 
 * @param header 
 * @param controlField 
 * @param type 
 * @param size 
 * @return unsigned char* 
 */
unsigned char COM_stateMachineHandler(enum state* connection_state, unsigned char byte_read)

/**
 * @brief Send Control Message
 * 
 * @param fd 
 * @param header 
 * @param controlField 
 */
void writeMessage(int port, unsigned char msg[])

void createTrama(unsigned char a, unsigned char controlField, unsigned char msg[]);
/**
 * @brief Stuffs data
 * 
 * @param buffer 
 * @param size 
 * @return unsigned char* 
 */
unsigned char *stuffingData(unsigned char *buffer, int *size);
/**
 * @brief Returns File size
 * 
 * @param file 
 * @return int 
 */
int getFileSize(FILE *file);
/**
 * @brief Calculates BCC2
 * 
 * @param buffer 
 * @param size 
 * @return unsigned char 
 */
unsigned char calculateBCC2(const unsigned char *buffer, unsigned int size);

/**
 * @brief 
 * 
 * @param fd 
 */
void closeConnection(int fd);