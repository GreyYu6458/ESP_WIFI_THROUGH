#ifndef _UDP_INIT_H_
#define _UDP_INIT_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "memory_pool.h"
#include "socket_init.h"
#include "msg_def.h"
#include "task.h"
#include "queue.h"
#include "config.h"
#include "esp_log.h"


typedef struct xUDPTask_t
{
    int hsocket;
    struct sockaddr_in to;
    struct sockaddr_in from;
    char rec_task_name[15];
    char send_task_name[15];

    xQueueHandle rec_queue;
    xQueueHandle send_queue;
    BaseType_t rec_task_handle;
    BaseType_t send_task_handle;
    UBaseType_t uxPriority;
    void (*RecCallback)(struct xUDPTask_t *, void *, void *);
    void* friend;
    xMemoryPoolHandle xMPool;
} xUDPTask_t;
typedef xUDPTask_t *xUDPTaskHandle;
typedef void (*xUDPRecCallBack_t)(xUDPTaskHandle, void *, void *);

extern xUDPTaskHandle xUDPTaskCreate(int sock, struct sockaddr_in *to, int16_t inQueueSize,
                                     int16_t outQueueSize, UBaseType_t uxPriority, xMemoryPoolHandle xMPool);

extern void xUDPTaskStart(xUDPTaskHandle xUDPTask);

extern void xUDPTaskSetRemote(xUDPTaskHandle xUDPTask, struct sockaddr_in *to);

extern void xUDPTaskSend(xUDPTaskHandle xUDPTask, const xMemoryBlockHandle xMB);

extern void xUDPTaskSetRecCallback(xUDPTaskHandle xUDPTask, xUDPRecCallBack_t xUDPRCb);

extern void xUDPTaskSetFriend(xUDPTaskHandle xUDPTask, void* friend);

#endif