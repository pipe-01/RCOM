#include "appLayer.h"


int llopen(int port, int role){
    struct termios oldtio, newtio;

    //open port
    int fd = open(port, O_RDWR | O_NOCTTY);
    if(fd < 0){
        perror(port);
        printf("Error opening port\n");
        exit(-1);
    }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
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

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    if(role == RECEIVER){ //receiver
        int *size = malloc(sizeof(int));

        stateMachine(fd, A_TRM, SET, S, size);

        printf("\nTrama SET recebida\n");
        sendControlMsg(fd, A_TRM, UA);
        printf("\nTrama UA enviada\n");
    }
    else if(role == TRANSMITTER){ //sender
        (void)signal(SIGALRM, alarmHandler);
        if(!setUpTransmitter(fd)){
            printf("\nCommunication protocol failed after %d tries\n", MAX_RETRY);
            return -1;
        }
    }

    return fd;
}


int llwrite(int fd, char * buffer, int length){
    return 0;
}

int llread(int fd, char * buffer){
    return 0;
}

int llclose(int fd){
    return 0;
}