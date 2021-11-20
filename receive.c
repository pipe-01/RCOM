#include "receive.h"


void setUpReceive(int fd){}

fileInfo receiveControlPackage(int fd)
{

    int *sizeControlPackage = malloc(sizeof(int));
    unsigned char *controlPackage = stateMachine(fd, A_TRM, 0x00, I, sizeControlPackage);

    fileInfo fileinfo;
    int controlPackageStatus = checkControlPackage(controlPackage, sizeControlPackage, &fileinfo);

    printf("\nTrama I de controlo recebida - STATUS: %x\n", controlPackageStatus);

    return fileinfo;
};