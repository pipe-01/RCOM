#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS5"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define I 1
#define S 0

#define MAX_RETRY 3

#define FLAG 0x7E
#define A_TRM 0x03
#define A_REC 0x01
#define SET 0x03
#define DISC 0x0B
#define UA 0x07
#define RR 0x05
#define REJ 0x01
#define SET_BCC A_TRM ^ SET
#define UA_BCC A_TRM ^ UA

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

unsigned char *stateMachine(int fd, unsigned char header, char controlField, int type, int *size);
void receiveControlMsg(int fd, unsigned char header, unsigned char controlField);
void sendControlMsg(int fd, unsigned char header, unsigned char controlField);
unsigned char *stuffingData(unsigned char *buffer, int *size);
int getFileSize(FILE *file);
unsigned char calculateBCC2(const unsigned char *buffer, unsigned int size);