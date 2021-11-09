
/*Non-Canonical Input Processing*/
#include "macros.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;



int receiveMessage(int fd, unsigned char msg){
  unsigned char * received_msg;
  int br = 0;
  while(br != 5){
    res = read(fd, &received_msg, 1);
    if(res > 0){
      switch (br){
      case 0: //FLAG
        if(receive_msg == FLAG){
          br = 1;
        }
        break;
      case 1: // A
        if(receive_msg == A){
          br = 2;
        }
        else{
          if(receive_msg == FLAG){
            br = 1;
          }
          else{
            br = 0;
          }
        }
        break;
      
      case 2: // C
        if(received_msg == msg){
          br = 3;
        }
        else{
          br = 0;
        }
        break;
      
      case 3: // BCC
        if(received_msg == (A ^ msg)){
          br = 4;
        }
        else{
          br = 0;
        }
        break;

      case 4: // FLAG
        if(received_msg == FLAG){
          br = 5;
        }
        else{
          br = 0;
        }
        break;
      
      default:
        break;
      }
    }
    else{
      return FALSE;
    }
  }
  return TRUE;
}

void resendMessage(int fd, unsigned char msg) {
  unsigned char send_msg[5];
  send_msg[0] = FLAG;
  send_msg[1] = A;
  send_msg[2] = msg;
  send_msg[3] = send_msg[1]^send_msg[2];
  send_msg[4] = FLAG;
  write(fd, send_msg, 5);
}

int main(int argc, char** argv)
{
    int fd,c, res, i=0;
    struct termios oldtio,newtio;
    char buf[255], aux[255]={};

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS5", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS5\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

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

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


    fflush(NULL);
    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);
      if(res > 0){   /* returns after 5 chars have been input */
                     /* so we can printf... */
         buf[res] = 0;
         printf("%s", buf);
            aux[i] = buf[0];
             if (aux[i]=='\0' || aux[i] == '\n') STOP=TRUE;
                    i++;
        }
        
    }
    printf("\n");
    

    if(receiveMessage(fd, SET)){
      resendMessage(fd, UA);
    }

    



  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */



    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
