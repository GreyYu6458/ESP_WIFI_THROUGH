#ifndef _CONFORM_H_
#define _CONFORM_H_
#include "main.h"

typedef void (*CallBack)(void *result);

extern void conform_service(char *c, int sc);

extern void conform_event_init(CallBack on_co1, CallBack on_co2, CallBack on_fa);

#endif