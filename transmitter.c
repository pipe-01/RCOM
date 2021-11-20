#include "transmitter.h"

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