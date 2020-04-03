#ifndef _UDP_INIT_H_
#define _UDP_INIT_H_
#include "main.h"
#include "msg_def.h"

typedef struct UDPObject
{
    UDPObject* this;
    int hsocket;
    const struct sockaddr *to;
    char* rec_task_name;
    char* send_task_name;


    xQueueHandle udp_rec_queue;
    xQueueHandle udp_send_queue;
    xTaskHandle rec_task_handle;
    xTaskHandle send_task_handle;

    void (*rec_task_start)(UDPObject* this);
    void (*write)(const transport_data *data, UDPObject* this);
    void (*send)(const transport_data* data, UDPObject* this);
    void (*rec_callback)(void* data, struct sockaddr_in* sourceAddr,UDPObject* this);
}UDPObject;

/*
* @brief            初始化UDP的连接
* @param s          socket句柄
* @param to         目标设备地址信息
*/
extern UDPObject* UDPObject_Consturct(int s, struct sockaddr *to);

/*
* @brief            构造UDPObject
* @param ob         UDPObject对象
*/
extern void UDPObject_Delete(UDPObject* ob);

#endif