#include "alarme.h"
#include "macros.h"


void alarmHandler(int signal){
   
   if(signal != SIGALRM){
      printf("Received signal: %d\n", signal); //debug
      return;
   }
   printf("\nAlarm: %d\n", alarmData.numRetry + 1);
   alarmData.alarmFlag = TRUE;
   alarmData.numRetry++;
}


void setupAlarm(){
   (void)signal(SIGALRM, alarmHandler);
   alarmData.alarmFlag = FALSE;
   alarmData.numRetry = 0;
   alarm(TIMEOUT);
}


void disableAlarm(){

   (void)signal(SIGALRM, SIG_IGN);
   alarm(0);
}

// int checkAlarm(){
//    if(!alarmFlag || numRetry != MAX_RETRY){
//       printf("\nTrama UA recebida\n");
//       alarmFlag = FALSE;
//       numRetry = 0;
//       printf("\nProtocol connection established!\n");
//       return TRUE;
//    }
//    else{
//       return FALSE;
//    }
//}

