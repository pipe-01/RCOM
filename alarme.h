#pragma once

#include <unistd.h>
#include <signal.h>
#include <stdio.h>

extern int numRetry;
extern int alarmFlag;

void alarmHandler();
int checkAlarm();