#ifndef _UART_H_
#define _UART_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "msg_def.h"
#include "memory_pool.h"
#include "driver/uart.h"

typedef struct UARTObject
{
    struct UARTObject* this;
    char rec_task_name[15];
    char send_task_name[15];
    int huart;
    
    memory_pool* mpool;
    xQueueHandle rec_queue;
    xQueueHandle send_queue;
    xTaskHandle rec_task_handle;
    xTaskHandle send_task_handle;
    
    void (*rec_task_start)(struct UARTObject* this);
    void (*send)(const indefiniteData* data, struct UARTObject* this);
    void (*rec_callback)(void* data, struct UARTObject* this);

}UARTObject;

extern UARTObject* UARTObject_Construct(uart_port_t u, uart_config_t uart_config, memory_pool* mpool);

extern void UARTObject_Delete(UARTObject* ob);

#endif