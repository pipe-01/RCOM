
/*Non-Canonical Input Processing*/

#include "macros.h"

volatile int STOP=FALSE;
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define MAX_BUF 255

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255], aux[255];
    int i, sum = 0, speed = 0;
    
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

    printf("New termios structure set\n");

    //printf("Write whatever you want: ");
    
    fgets(buf, 255, stdin);

    int nchar = 0;
    for(int i=0; i < 256; i++){
      if(buf[i]=='\0'){
        nchar = i;
        break;
      }
        
    }
   
    res = write(fd,buf,nchar);   
    printf("%d characters written\n", nchar);

  /** 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  **/

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    
    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);
      if(res == -1){
        printf("Error\n");
        STOP = TRUE;
      }
      else if(res > 0){   /* returns after 5 chars have been input */
                     /* so we can printf... */
         buf[res] = 0;
         printf("%s", buf);
            aux[i] = buf[0];
             if (aux[i]=='\0' || aux[i] == '\n'){
              STOP=TRUE;
              printf("Im leaving\n");
             }
              i++;
        }
    }
    printf("Received the string: %s\n", aux);

  sleep(1);

    close(fd);
    return 0;
}
