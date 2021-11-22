#include "receive.h"


void setUpReceive(int fd){}

int checkControlPackage(unsigned char *controlPackage, int *size, fileInfo *fileinfo)
{
    //Filesize field
    if (controlPackage[1] == 0)
    {
        int fileSize = 0;
        int shift = 24;
        for (int i = 3; i < controlPackage[2] + 3; i++)
        {
            fileSize |= (int)controlPackage[i] << shift;
            shift -= 8;
        }
        fileinfo->size = fileSize;
    }

    //Filename field
    if (controlPackage[7] == 1)
    {
        unsigned char *fileName = malloc(sizeof(unsigned char) * controlPackage[8]);
        int counter = 0;

        for (int i = 9; i < controlPackage[8] + 9; i++)
        {
            int nameSize = 0;
            fileName[counter++] = controlPackage[i];
        }

        fileinfo->filename = fileName;
    }

    return controlPackage[0];
}

fileInfo receiveControlPackage(int fd)
{

    int *sizeControlPackage = malloc(sizeof(int));
    unsigned char *controlPackage = stateMachine(fd, A_TRM, 0x00, I, sizeControlPackage);

    fileInfo fileinfo;
    int controlPackageStatus = checkControlPackage(controlPackage, sizeControlPackage, &fileinfo);

    printf("\nTrama I de controlo recebida - STATUS: %x\n", controlPackageStatus);

    return fileinfo;
}

void handleDisconnection(int fd)
{
    int *size = malloc(sizeof(int));

    stateMachine(fd, A_TRM, DISC, S, size);
    printf("Trama DISC recebida!\n");
    sendControlMsg(fd, A_REC, DISC);
    printf("Trama DISC enviada!\n");

    stateMachine(fd, A_REC, UA, S, size);
    printf("Trama UA recebida!\n");

    printf("Connection closed!\n");
}