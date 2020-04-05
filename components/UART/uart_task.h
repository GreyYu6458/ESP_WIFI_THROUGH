#ifndef _UART_H_
#define _UART_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "msg_def.h"
#include "memory_pool.h"
#include "uart_init.h"

typedef struct xUARTTask_t
{
    int unum;
    
    char rec_task_name[15];
    char send_task_name[15];

    xQueueHandle rec_queue;
    xQueueHandle send_queue;
    xTaskHandle rec_task_handle;
    xTaskHandle send_task_handle;
    UBaseType_t uxPriority;
    void (*RecCallback)(struct xUARTTask_t *, void *);

    xMemoryPoolHandle xMPool;
} xUARTTask_t;
typedef xUARTTask_t *xUARTTaskHandle;
typedef void (*xUARTRecCallBack_t)(xUARTTaskHandle, void *);

extern xUARTTaskHandle xUARTTaskCreate(uart_port_t p, int16_t inQueueSize, int16_t outQueueSize, 
                                       UBaseType_t uxPriority, xMemoryPoolHandle xMPool);

extern void xUARTTaskStart(xUARTTaskHandle xUARTTask);

extern void xUARTTaskSend(xUARTTaskHandle xUARTTask, const xMemoryBlockHandle xMB);

extern void xUARTTaskSetRecCallback(xUARTTaskHandle xUARTTask, xUARTRecCallBack_t xUARTRCb);

#endif