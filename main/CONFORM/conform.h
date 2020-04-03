#ifndef _CONFORM_H_
#define _CONFORM_H_
#include "main.h"
#include "msg_def.h"
#include "conform_event.h"

enum conform_status
{
    LINK_FIRST = 0,
    LINK_SECOND,
    LINKING
};

extern void udp_conform_callback(void* data, struct sockaddr_in* sourceAddr,UDPObject* this);

#endif