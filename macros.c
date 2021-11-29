#include "macros.h"

void createTrama(unsigned char a, unsigned char controlField, unsigned char msg[]){
    msg[5] = {FLAG, a, controlField, a ^ controlField, FLAG};
}


void writeMessage(int port, unsigned char msg[])
{
    int res = write(port, msg, sizeof(msg));
    if(res < 0){
        printf("Error writing\n");
        return;
    }
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


unsigned char COM_stateMachineHandler(State_Machine *current_state, unsigned char byte_read) {
	static unsigned char control_byte = 0;

	switch(*current_state) {
		case START: {
			if(byte_read == FLAG){
				*current_state = FLAG_RCV;
            }
            else{
                *current_state = START;
            }
			break;
		}
		case FLAG_RCV: {
			if(byte_read == A_CMD){
				*current_state = A_RCV;
            }
			else
            {
                if(byte_read == FLAG){
                    *current_state = FLAG_RCV;
                }
                else{
                    *current_state = START;
                }
            }
			break;
		}
		case A_RCV: {
			if ((byte_read == CONTROL_SET) || (byte_read == CONTROL_UA) || (byte_read = CONTROL_DISC) || (byte_read == C0) || (byte_read == C1)) {
				*current_state = C_RCV;
				control_byte = byte_read;
			}
			else if (byte_read == FLAG){
				*current_state = FLAG_RCV;
            }
			else{
				*current_state = START;
            }
			break;
		}
		case C_RCV: {
			if (byte_read == (BCC(A_CMD, control_byte))){
				*current_state = BCC_OK;
            }
			else if (byte_read == FLAG){
				*current_state = FLAG_RCV;
            }
			else{
				*current_state = START;
            }
			break;
		}
		case BCC_OK: {
			if (byte_read == FLAG) {
				*current_state = STOP;
			}
			else
				*current_state = START;
			break;
		}
		default:
			break;
	}
	return control_byte;
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


void closeConnection(int fd)
{
    //close fd and set old termios struct
   sleep(2);
   if(tcsetattr(fd, TCSANOW, &oldtio) == -1){
       perror("tcsetattr\n");
       return;
   }
   close(fd);
}