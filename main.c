#include "macros.h"
#include "appLayer.h"
#include <time.h>

int main(int argc, char **argv)
{
    int status;
    if (argc == 3)
    {
        status = RECEIVER;
    }
    else if (argc == 4)
    {
        status = TRANSMITTER;
    }
    else
    {
        printf("Usage:\n");
        printf("\tTRANSMITTER: ./main /dev/sttyS0 send <filename>\n");
        printf("\tRECEIVER: ./main /dev/sstyS1 receive\n");
        return -1;
    }

    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */
    int fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(argv[1]);
        exit(-1);
    }

    FILE *file;
    if (status == TRANSMITTER)
    {
        if (!(file = fopen(argv[3], "rb")))
        {
            printf("Error opening the file or the file does not exist!\n");
            return (-1);
        }
        else
        {
            printf("Reading file...\n"); //debug
        }
    }

    if (llopen(fd, status) == -1){
        return -1;
    }

    clock_t start = clock();
    /*Do something*/
    switch (status){

    case TRANSMITTER:
        llwrite(fd, file, argv[3]);
        break;
    case RECEIVER:
        llread(fd);
        break;

    default:
        return -1;
        break;
    }
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;

    printf("\nTime spent: %f\n", seconds);

    llclose(fd, status);

    return 0;
}