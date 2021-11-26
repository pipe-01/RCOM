#include "appLayer.h"

const unsigned char SET[] = {FLAG, A_TRM, C_SET, BCC(A_TRM, C_SET),FLAG};
const unsigned char UA[] = {FLAG, A_TRM, C_UA, BCC(A_TRM, C_UA), FLAG};
const unsigned char DISC[] = {FLAG, A_TRM, C_DISC, BCC(A_TRM, C_DISC), FLAG};
const unsigned char RR0[] = {FLAG, A_TRM, C_RR0, BCC(A_TRM, C_RR0), FLAG};
const unsigned char RR1[] = {FLAG, A_TRM, C_RR1, BCC(A_TRM, C_RR1), FLAG};
const unsigned char REJ0[] = {FLAG, A_TRM, C_REJ0, BCC(A_TRM, C_REJ0), FLAG};
const unsigned char REJ1[] = {FLAG, A_TRM, C_REJ1, BCC(A_TRM, C_REJ1), FLAG};

int llopen(int port, int role){
    struct termios oldtio, newtio;

    if ( tcgetattr(port,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */

    /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(port, TCIOFLUSH);

    if ( tcsetattr(port, TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    if(role == TRANSMITTER){
        //send SET
        printf("[STARTING CONNECTION]\n");

        do{
            printf("[SENDING SET]\n");
            sendControlMsg(port, SET); //send SET message
            setupAlarm();              //install alarm
            printf("[READING UA]\n");
            readMessage(port, UA);

        } while(alarmData.alarmFlag == FALSE && alarmData.numRetry <= MAX_RETRY);

        if(alarmData.numRetry >= MAX_RETRY){
            printf("Numero de tentativas excedido\n"); //debug
            return -1;
        }
        alarmData.numRetry = 0;
        disableAlarm(); //disable alarm
    }
    else if(role == RECEIVER){
        printf("[READING SET]\n");
        readMessage(port, SET);
        printf("[SENDING UA]\n");
        sendControlMsg(port, UA);
    }
    else{
        printf("Invalid role: %d\n", role); //debug
        return -1;
    }

    return port;
}


int llwrite(int port, FILE *f, char *fName)
{
    int fSize = getFileSize(f);

    unsigned char *buffer = malloc(sizeof(unsigned char) * fSize);

    fread(buffer, sizeof(unsigned char), fSize, f);

    int seqN = sendControl(port, fSize, fName, 0x02);

    sendData(port, buffer, fSize, seqN);

    sendControl(port, fSize, fName, 0x03);

    return 0;
}

int llread(int port)
{
    fileInfo dataInfo = receiveControlPackage(port);

    int numFrame = dataInfo.size / 256;

    int l1 = dataInfo.size / 256;
    int l2 = dataInfo.size % 256;

    if (l2 != 0)
    {
        numFrame++;
    }

    printf("Number of Frames: %d", numFrame); //debug
    int *size = malloc(sizeof(int));

    unsigned char *fileData = malloc(sizeof(unsigned char) * dataInfo.size);

    int counter = 0;
    int n = -1;
    int currentN = 0;
    int fail = FALSE;
    for (int i = 0; i < numFrame; i++)
    {

        if (fail == TRUE)
        {
            printf("numFrame: %d\n failed at - %d\n", numFrame, i);
        }

        unsigned char *data = stateMachine(port, A_TRM, 0x00, I, size);
        currentN = data[1];

        if (currentN == n + 1)
        {
            if (i < numFrame - 1 && (*size >= 255 && *size <= 261))
            {
                int cnt = 0;
                for (int d = 4; d < (*size) - 1; d++)
                {
                    cnt++;
                    fileData[counter++] = data[d];
                }
                printf("FRAME - %d | SIZE: %d\n", data[1], cnt);
                fail = FALSE;
                n = currentN;
            }
            else if (i == numFrame - 1 && (*size >= l2 && *size <= l2 + 5))
            {
                int cnt = 0;
                for (int d = 4; d < (*size) - 1; d++)
                {
                    cnt++;
                    fileData[counter++] = data[d];
                }
                printf("LAST FRAME - %d | SIZE: %d\n", data[1], cnt);
                n = currentN;
            }
            else if (*size > 261)
            {
                int cnt = 0;
                for (int d = (*size) - 257; d < (*size) - 1; d++)
                {
                    cnt++;
                    fileData[counter++] = data[d];
                }
                printf("FRAME - %d | SIZE: %d\n", data[1], cnt);
                fail = FALSE;
                n = currentN;
            }
            else
            {
                printf("%d\n", *size);
                printf("Incorrect frame with i = %d\n", i);
                //  sendControlMsg(port, A_TRM, 0x01);
                i--;
                printf("Decreased i: %d\n", i);
            }
        }
        else if (currentN == n)
        { //duplicado
            printf("Received Duplicate with i = %d\n", i);
            i--;
            printf("Decreased i by: %d\n", i);
        }
        else
        {
            i--;
        }

        if (currentN == 255)
        {
            n = -1;
        }
    }

    printf("SIZE: %d", counter);

    fileInfo dataInfoFinal = receiveControlPackage(port);

    //createFile(dataInfo, fileData);

    //create file
    char path[9 + strlen(dataInfo.filename)];
    strcpy(path, "received_");
    strcat(path, dataInfo.filename);
    printf("PATH: %s\n", path);
    FILE *received_file = fopen(path, "w+b");

    if(received_file == NULL){
        printf("Error creating file %s\n", path);
        return -1;
    }

    fwrite((void *) fileData, 1, dataInfo.size, received_file);
    fclose(received_file);

    return 0;
}



int llclose(int fd, int status){
    if (status == TRANSMITTER)
    {
        closeConnection(fd);
    }
    if (status == RECEIVER)
    {
        handleDisconnection(fd);
    }
    close(fd);

    return 0;
}