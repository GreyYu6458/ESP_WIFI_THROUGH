#ifndef _CONFORM_EVENT_H_
#define _CONFORM_EVENT_H_
#include "main.h"
#include "conform.h"

extern char on_con_1(void* result, struct sockaddr_in* sourceAddr);
extern char on_con_2(void* result, struct sockaddr_in* sourceAddr);
extern char on_linking(void* result, struct sockaddr_in* sourceAddr);
extern char on_fai(void* result, struct sockaddr_in* sourceAddr);

#endif