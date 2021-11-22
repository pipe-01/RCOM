#pragma once

#include <unistd.h>
#include <signal.h>
#include <stdio.h>


extern int numRetry;
extern int alarmFlag;

/**
 * @brief Prints out number of tests to alarm
 * 
 */
void alarmHandler();
/**
 * @brief Handles alarm signal and runs when alarm is triggered
 * 
 * @return True if Protocol Connection Established, else False
 */
int checkAlarm();