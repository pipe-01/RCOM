#include "appLayer.h"

const unsigned char SET[] = {FLAG, A_TRM, C_SET, BCC(A_TRM, C_SET),FLAG};
const unsigned char UA[] = {FLAG, A_TRM, C_UA, BCC(A_TRM, C_UA), FLAG};
const unsigned char DISC[] = {FLAG, A_TRM, C_DISC, BCC(A_TRM, C_DISC), FLAG};
const unsigned char RR0[] = {FLAG, A_TRM, C_RR0, BCC(A_TRM, C_RR0), FLAG};
const unsigned char RR1[] = {FLAG, A_TRM, C_RR1, BCC(A_TRM, C_RR1), FLAG};
const unsigned char REJ0[] = {FLAG, A_TRM, C_REJ0, BCC(A_TRM, C_REJ0), FLAG};
const unsigned char REJ1[] = {FLAG, A_TRM, C_REJ1, BCC(A_TRM, C_REJ1), FLAG};

Data_control data_control;
State_Machine current_state;
AlarmData alarmData;

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

    data_control.N_s = 0;

    if(role == TRANSMITTER){
        //send SET
        printf("[STARTING CONNECTION]\n");

        do{
            printf("Sending SET]\n");
            writeMessage(port, SET); //send SET message
            setupAlarm();              //install alarm
            printf("Reading UA]\n");
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
        printf("Reading SET\n");
        readMessage(port, SET);
        printf("Sending UA\n");
        writeMessage(port, UA);
    }
    else{
        printf("Invalid role: %d\n", role); //debug
        return -1;
    }

    return port;
}


int llwrite(int port, unsigned char *packet, int packet_size)
{
    unsigned int frame [2 * packet_size + 6]; //6 -> F + A + BCC1 + BCC2 + F | 2 * packet para assegurar espaço suficiente para byte stuffing
    unsigned int framePosition;

    if(data_control.N_s == 0){
        data_control.N_s = 1;
    }
    else{
        data_control.N_s = 0;
    }


    //Frame: [F, A, C, BCC1, PACKET, BCC2, F]
    framePosition = 4;
    //Set of frame header
    frame[0] = FLAG;
    frame[1] = A_CMD;
    frame[2] = (data_control.N_s == 0 ? C0 : C1);
    frame[3] = BCC(frame[1], frame[2]);

    //process data
    unsigned int dataIndex = 0;
    unsigned char currPacket_char;

    while(dataIndex < packet_size){
        currPacket_char = packet[dataIndex++];

        //Byte stuffing
        if(currPacket_char == FLAG || currPacket_char == ESCAPEMENT){
            frame[framePosition++] = ESCAPEMENT;
            frame[framePosition++] = currPacket_char ^ BYTE_STUFFING
        }
        else{
            frame[framePosition++] = currPacket_char;
        }
    }

    unsigned char bcc2 = calculateBCC2(packet, packet_size);

    // Set of frame footer
    if(bcc2 == ESCAPEMENT || bcc2 == FLAG){
        frame[framePosition++] = ESCAPEMENT;
        frame[framePosition++] = bcc2 ^ BYTE_STUFFING;
    }
    else{
        frame[framePosition++] = bcc2;
    }

    frame[framePosition++] = FLAG;
    
    int res;

    do{
        res = write(fd, frame, framePosition);

        setupAlarm();

        if(readResponse(fd) == -1){
            disableAlarm();
            data_control.RJreceived++;
            printf("Received REJ #%d\n", data_control.RJreceived);
            continue;
        }    
    } while (alarmData.alarmFlag && alarmData.numRetry <= MAX_RETRY);

    if(alarmData.numRetry >= MAX_RETRY){
        printf("Numero de tentativas excedido\n"); //debug
        return -1;
    }
    else{
        alarmData.numRetry = 1;
    }
    disableAlarm(); //disable alarm
    
    printf("Sent frame number %d with size %d\n", data_control.framesSent, framePosition);
    data_control.framesSent++;
    
    return res;
}

int llread(int port, unsigned char *pakcet, int stage)
{
    unsigned char frame[MAX_FRAME_SIZE];
    int received = FALSE;
    int frame_length = 0;
    unsigned char control_field;


    while(!done){
        frame_length = readFrame(fd, frame);
        if(frame_length == -1){
            return -1;
        }

        //destuff packet
        unsigned char final_frame[frame_length];
        int final_frame_length = destuffFrame(frame, frame_length, final_frame);  
        control_field = frame[2];   
           
    }
}



int llclose(int fd, int status){
    if (status == TRANSMITTER)
    {
        printf("CLOSING CONNECTION\n Sending DISC\n");
        do
        {
            //send DISC
            writeMessage(fd, DISC);
            setupAlarm();
            //read DISC
            printf("Reading Disc\n");
            readMessage(fd, UA);
            disableAlarm();
        } while (alarmData.alarmFlag && alarmData.numRetry <= MAX_RETRY);

        if(alarmData.numRetry >= MAX_RETRY){
            printf("MAX TRIES!\n");
            return -1;
        }
        else{
            alarmData.numRetry = 1;
        }
        disableAlarm();
        printf("UA received\n");
        
    }
    else if (status == RECEIVER)
    {
        //receive DISC
        setupAlarm();
        readMessage(fd, DISC);
        disableAlarm();
        printf("DISC received\n");
        //send UA
        printf("Sending UA\n");
        writeMessage(fd, UA);
    }
    else{
        printf("Unknow status: %d\n", status);
        return 1;
    }
    closeConnection(fd);

    return 0;
}

int readMessage(int port, unsigned char trama[]){
    current_state = START;
    unsigned char byte_read;

    while(current_state != STOP){
        read(port, &byte_read, 1);
        COM_stateMachineHandler(&current, byte_read);
    }
}

int readResponse(int fd){
    unsigned char byte_read, control_field;
    current_state = START;
    int res = 0;
    while(current_state != STOP && !alarmData.alarmFlag){
        res = read(fd, &byte_read, 1);
        COM_stateMachineHandler(&current, byte_read);
        if(current_state == BCC_OK){
            control_field = byte_read;
        }
    }
    if(control_field == C_R0 && data_control.N_s == 1){
        return 0;
    }
    else if(control_field == C_R1 && data_control.N_s == 0){
        return 0;
    }
    else{
        return -1;
    }
}