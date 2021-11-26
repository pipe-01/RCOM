#include "transmitter.h"

int numRetry = 0;
int alarmFlag = FALSE;

int setUpTransmitter(int fd){
    do{
        sendControlMsg(fd, A_TRM, SET);
        printf("\nTrama SET enviada\n");

        alarmFlag = FALSE;
        alarm(TIMEOUT);

        int *size = malloc(sizeof(int));
        stateMachine(fd, A_TRM, UA, S, size);

    } while(alarmFlag && numRetry < MAX_RETRY);

    return checkAlarm();
}
void sendControlPackage(int port, unsigned char *controlPackage, int *size, unsigned char bcc2, int s)
{
    int bufferSize = *size + 6;
    unsigned char buffer[bufferSize];

    int counter = 0;
    buffer[counter++] = FLAG;
    buffer[counter++] = A_TRM;
    if (s == 0)
    {
        buffer[counter++] = 0x00;
    }
    else
    {
        buffer[counter++] = 0x40;
    }
    buffer[counter++] = buffer[1] ^ buffer[2]; //bcc

    //Envio Control Package
    for (int i = 0; i < (*size); i++)
    {
        buffer[counter++] = controlPackage[i];
    }

    buffer[counter++] = FLAG;

    write(port, &buffer, bufferSize);
}

unsigned char *generateControlPackage(int fSize, unsigned char *fName, int *packSize, int controlField)
{
    int sizefName = strlen(fName);
    int packSizeAux = 9 * sizeof(unsigned char) + sizefName;
    unsigned char *controlPackage = malloc(sizeof(unsigned char) * (packSizeAux + 1));

    controlPackage[0] = controlField;
    controlPackage[1] = T1; //file size
    controlPackage[2] = L1;
    controlPackage[3] = (fSize >> 24) & 0xFF;
    controlPackage[4] = (fSize >> 16) & 0xFF;
    controlPackage[5] = (fSize >> 8) & 0xFF;
    controlPackage[6] = (fSize & 0xFF);
    controlPackage[7] = T2; //fName
    controlPackage[8] = sizefName;
    for (int i = 0; i < sizefName; i++)
    {
        controlPackage[9 + i] = fName[i];
    }

    *packSize = packSizeAux + 1;

    return controlPackage;
}
void sendData(int port, unsigned char *buffer, int size, int seqN)
{
    //Cálculo nr tramas
    int nTramas;

    int l1 = size / 256;
    int l2 = size % 256;

    nTramas = l1;
    if (l2 != 0)
    {
        nTramas++;
    }

    unsigned char info[MAX_SIZE];

    info[0] = FLAG;
    info[1] = A_TRM;

    for (int i = 0; i < nTramas; i++)
    {
        (void)signal(SIGALRM, alarmHandler);
        numRetry = 0;

        do
        {
            int counter = 2;

            //send
            if (seqN == 0)
            {
                info[counter++] = 0x00;
            }
            else
            {
                info[counter++] = 0x40;
            }

            //BCC
            info[counter++] = info[1] ^ info[2];

            int *dataPackageSize = malloc(sizeof(int));
            *dataPackageSize = size;

            unsigned char *dataPackage = generateDataPackage(buffer, dataPackageSize, i, l1, l2);

            //BCC2
            unsigned char bcc2 = calculateBCC2(dataPackage, *dataPackageSize - 1);

            dataPackage[*dataPackageSize - 1] = bcc2;

            //stuffing
            unsigned char *stuffedData = stuffingData(dataPackage, dataPackageSize);

            //data
            for (int j = 0; j < (*dataPackageSize); j++)
            {
                info[counter++] = stuffedData[j];
            }

            info[counter++] = FLAG;

            alarmFlag = FALSE;
            alarm(TIMEOUT);

            write(port, &info, counter);

            int *size = malloc(sizeof(int));

            int c_state;

            if (seqN == 0)
            {
                c_state = 0x05; //Expects positive ACK -> controlField val = 0x05 (R = 0)
            }
            else
            {
                c_state = 0x85; //Expects positive ACK -> controlField val = 0x85 (R = 1)
            }

            unsigned char *status = stateMachine(port, A_TRM, c_state, S, size);
            printf("Status: %x\n", status[0]);
            if (status[0] == 0x0)
            {
                printf("Trama RR recebida!\n");
                break;
            }
            else if (status[0] == 0x1)
            {
                printf("Trama RJ recebida - send Data!\n");
                printf("ALARM FLAG %d numretry %d", alarmFlag, numRetry);
            }
            else
            {
                printf("Waiting.... \n");
            }

            (seqN == 0) ? seqN++ : seqN--;

        } while (alarmFlag && numRetry < MAX_RETRY);
        if (alarmFlag && numRetry == MAX_RETRY)
            break;

        (seqN == 0) ? seqN++ : seqN--;
    }

    if (alarmFlag && numRetry == MAX_RETRY)
        return;

    else
    {
        alarmFlag = FALSE;
        numRetry = 0;
        return;
    }
}
int sendControl(int port, int fSize, unsigned char *fName, int controlField)
{
    int *size = malloc(sizeof(int));
    unsigned char *controlPackage = generateControlPackage(fSize, fName, size, controlField);

    unsigned char bcc2 = calculateBCC2(controlPackage, *size - 1);

    controlPackage[*size - 1] = bcc2;

    unsigned char *stuffedControlPackage = stuffingData(controlPackage, size);

    int seqN = 0;
    do
    {
        sendControlPackage(port, stuffedControlPackage, size, bcc2, seqN);

        alarmFlag = FALSE;
        alarm(TIMEOUT);

        int *size = malloc(sizeof(int));

        int c_state;

        if (seqN == 0)
        {
            c_state = 0x05; //Expects positive ACK -> controlField val = 0x05 (R = 0)
        }
        else
        {
            c_state = 0x85; //Expects positive ACK -> controlField val = 0x85 (R = 1)
        }

        unsigned char *status = stateMachine(port, A_TRM, c_state, S, size);
        if (status[0] == 0x0)
        {
            printf("Trama RR recebida!\n");
            break;
        }
        else if (status[0] == 0x1)
        {
            printf("Trama RJ recebida - send Control!\n");
        }

        (seqN == 0) ? seqN++ : seqN--;

    } while (alarmFlag && numRetry < MAX_RETRY);

    if (alarmFlag && numRetry == MAX_RETRY)
        return -1;

    else
    {
        printf("\nTrama I de controlo enviada!\n");
        alarmFlag = FALSE;
        numRetry = 0;
        return seqN;
    }
}

unsigned char *generateDataPackage(unsigned char *buf, int *size, int n, int l1, int l2)
{
    unsigned char dataPackage[*size];

    dataPackage[0] = C_DATA;
    dataPackage[1] = n;
    dataPackage[2] = l1;
    dataPackage[3] = l2;

    int counter = 4;
    int dataSize = 4;
    //ultima trama

    if (n == l1)
    {
        for (int i = n * 256; i < n * 256 + l2; i++)
        {
            dataPackage[counter++] = buf[i];
        }
        dataSize += l2;
    }
    else
    {
        for (int i = n * 256; i < n * 256 + 256; i++)
        {
            dataPackage[counter++] = buf[i];
        }
        dataSize += 256;
    }

    int cnt = 0;
    unsigned char *dp = malloc((4 + dataSize) * sizeof(unsigned char));

    for (int i = 0; i < 4 + dataSize; i++)
    {
        dp[cnt++] = dataPackage[i];
    }

    *size = dataSize + 1;

    return dp;
}

void closeConnection(int fd)
{
    do
    {
        sendControlMsg(fd, A_TRM, SET);
        //printf("\nTrama DISC enviada\n");

        alarmFlag = FALSE;
        alarm(TIMEOUT);

        int *size = malloc(sizeof(int));
        stateMachine(fd, A_REC, DISC, S, size);
        //printf("Trama DISC recebida!\n");

        sendControlMsg(fd, A_REC, UA);
        //printf("Trama UA enviada!\n");

    } while (alarmFlag && numRetry < MAX_RETRY);

    if (alarmFlag && numRetry == MAX_RETRY)
        return;

    else
    {
        printf("Connection closed!\n");
        alarmFlag = FALSE;
        numRetry = 0;
        return;
    }
}
