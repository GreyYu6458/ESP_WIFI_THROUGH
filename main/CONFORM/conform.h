#ifndef _CONFORM_H_
#define _CONFORM_H_
#include "main.h"
#include "msg_def.h"

enum conform_status
{
    LINK_FIRST = 0,
    LINK_SECOND,
    LINKING
};

typedef struct 
{
    AuthenticateTask* this;
    UDPObject* udp_adapter;
    
    void (*authenticate_start)(AuthenticateTask* this);
}AuthenticateTask;


/*
* @brief            服务器验证类
* @param adapter    目标设备地址信息
*/
extern AuthenticateTask* AuthenticateTask_Consturct(UDPObject* adapter);


extern void send_device_detail(UDPObject* udpObject);

#endif