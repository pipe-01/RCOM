#include "alarme.h"
#include "macros.h"


void alarmHandler(){
   printf("\nAlarm: %d\n", numRetry + 1);
   alarmFlag = TRUE;
   numRetry++;
}

int checkAlarm(){
   if(!alarmFlag || numRetry != MAX_RETRY){
      printf("\nTrama UA recebida\n");
      alarmFlag = FALSE;
      numRetry = 0;
      printf("\nProtocol connection established!\n");
      return TRUE;
   }
   else{
      return FALSE;
   }
}

