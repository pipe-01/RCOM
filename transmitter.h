#pragma once

#include "macros.h"
#include <fcntl.h>
#include "alarme.h"

#define TIMEOUT 5

extern int numRetry;
extern int alarmFlag;

int setUpTransmitter(int fd);