#ifndef COMMON_H
#define COMMON_H

typedef int bool;
#define true 1
#define false 0

#include "tftp.h"

size_t charncpy(char *dest, const char *src, size_t n);

u_int16_t getHostOrderShortFromNetwork(void * buff);
u_int16_t getNetworkOrderShortFromHost(u_int16_t hostshort, void * buff);

#endif