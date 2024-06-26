#include "macros.h"

void sendControlMsg(int port, unsigned char header, unsigned char controlField)
{
    unsigned char msg[5];
    msg[0] = FLAG;
    msg[1] = header;
    msg[2] = controlField;
    msg[3] = (msg[1] ^ msg[2]);
    msg[4] = FLAG;
    write(port, msg, 5);
}

unsigned char *destuffingData(unsigned char *buf, int *size)
{
    int counter = 0;
    unsigned char destuffedData[MAX_SIZE];

    for (int i = 0; i < (*size); i++)
    {
        if (buf[i] == 0x7d)
        {
            if (buf[i + 1] == 0x5e)
            {
                destuffedData[counter++] = 0x7e;
            }
            else if (buf[i + 1] == 0x5d)
            {
                destuffedData[counter++] = 0x7d;
            }
            i++;
        }
        else
        {
            destuffedData[counter++] = buf[i];
        }
    }

    unsigned char *db = malloc(sizeof(unsigned char) * counter);

    for (int j = 0; j < counter; j++)
    {
        db[j] = destuffedData[j];
    }

    *size = counter;

    return db;
}


unsigned char *stateMachine(int port, unsigned char header, char controlField, int type, int *size)
{

    State_Machine state = START;
    unsigned char *msg = malloc(sizeof(unsigned char) * MAX_SIZE);
    unsigned char *res = malloc(sizeof(unsigned char));
    res[0] = 0x03;
    unsigned char c;
    int counter = 0;
    int seqN = 0;

    while (state != STOP && !alarmFlag)
    {
        read(port, &c, 1);

        switch (state)
        {
        case START:
            if (c == FLAG)
            {
                state = FLAG_RCV;
            }
            break;
        case FLAG_RCV:
            counter = 0;
            if (c == header)
            {
                state = A_RCV;
            }
            else
            {
                if (c == FLAG)
                    state = FLAG_RCV;
                else
                    state = STOP;
            }
            break;
        case A_RCV:
            if (type == FALSE)
            {
                if (c == controlField || c == 0x85 || c == 0x05)
                {
                    state = C_RCV;
                    res[0] = 0x0;
                }
                else
                {
                    if (c == FLAG)
                    {
                        state = FLAG_RCV;
                        res[0] = 0x1;
                        return res;
                    }
                
                    else
                    {
                        res[0] = 0x0;
                        return res;
                    }
                }
            }
            else if (type == TRUE)
            {
                if (c == 0x00)
                {
                    controlField = 0x00;
                    seqN = 0;
                    state = C_RCV;
                }
                else if (c == 0x40)
                {
                    seqN = 1;
                    state = C_RCV;
                    controlField = 0x40;
                }
                else
                {
                    if (c == FLAG)
                        state = FLAG_RCV;
                    else
                        state = START;
                }
            }

            break;
        case C_RCV:
            if (c == (A_TRM ^ controlField) || c == (A_TRM ^ 0x05) || c == (A_TRM ^ 0x85)) //BCC = A_TRM ^ C
            {
                state = BCC_OK;
            }
            else
                state = START;
            break;
        case BCC_OK:
            if (c == FLAG)
            {
                if (type == TRUE)
                {

                    *size = counter;

                    msg = destuffingData(msg, size);

                    unsigned char bcc2 = msg[*size - 1];

                    int sizeBcc = *size - 1;
                    unsigned char calcBcc2 = calculateBCC2(msg, sizeBcc);

                    unsigned char positiveACK;
                    unsigned char negativeACK; 
                    if (bcc2 == calcBcc2)
                    {

                        if (seqN == 0)
                        {
                            positiveACK = 0x05;
                        }
                        else
                        {
                            positiveACK = 0x85;
                        }
                        sendControlMsg(port, A_TRM, positiveACK);

                    }
                }
                state = STOP;
            }
            else
            {
                if (type == FALSE)
                {
                    state = START;
                }
                else
                {
                    msg[counter++] = c;
                    if (counter == MAX_SIZE)
                    {
                        counter = 0;
                        state = START;
                        free(msg);
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    if (type == TRUE)
    {
        unsigned char *data = malloc(sizeof(unsigned char) * (*size));
        for (int i = 0; i < (*size); i++)
        {
            data[i] = msg[i];
        }
        return data;
    }
    else
    {
        return res;
    }
}


unsigned char calculateBCC2(const unsigned char *buf, unsigned int size)
{
    unsigned char bcc2 = 0;

    for (unsigned int i = 0; i < size; i++)
    {
        bcc2 ^= buf[i];
    }
    return bcc2;
}

int stuffedSizeCalculator(unsigned char *buf, int size)
{
    int counter = 0;

    for (int i = 0; i < size; i++)
    {
        if (buf[i] == 0x7e)
        {
            counter++;
        }
        else if (buf[i] == 0x7d)
        {
            counter++;
        }
        counter++;
    }
    return counter;
}

unsigned char *stuffingData(unsigned char *buf, int *size)
{
    int startStuffedSize = *size;
    if (*size < MAX_SIZE)
        startStuffedSize = MAX_SIZE;

    int counter = 0;
    unsigned char stuffedBuffer[startStuffedSize];

    for (int i = 0; i < (*size); i++)
    {
        if (buf[i] == FLAG)
        {
            stuffedBuffer[counter++] = ESCAPEMENT;
            stuffedBuffer[counter++] = REPLACE_FLAG;
        }
        else if (buf[i] == ESCAPEMENT)
        {
            stuffedBuffer[counter++] = ESCAPEMENT;
            stuffedBuffer[counter++] = REPLACE_ESCAPEMENT;
        }
        else
        {
            stuffedBuffer[counter++] = buf[i];
        }
    }

    *size = counter;

    unsigned char *sb = malloc(sizeof(unsigned char) * (counter));
    for (int i = 0; i < counter; i++)
    {
        sb[i] = stuffedBuffer[i];
    }

    return sb;
}



int getFileSize(FILE *file)
{
    // guarda posição atual
    long int currentPosition = ftell(file);

    // procura eof
    if (fseek(file, 0, SEEK_END) == -1)
    {
        printf("ERROR: Could not get file size.\n");
        return -1;
    }

    // guarda tamanho do ficheiro
    long int size = ftell(file);

    // procura a posição guardada previamente
    fseek(file, 0, currentPosition);

    // retorna tamanho
    return size;
}
