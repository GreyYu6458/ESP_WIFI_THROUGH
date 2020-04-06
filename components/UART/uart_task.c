#include "uart_task.h"

static void uart_rec_task(void *parameter);
static void uart_send_task(void *parameter);
static void xUARTRecCallBack(xUARTTaskHandle xUARTTask, void *data, void* friend);

xUARTTaskHandle xUARTTaskCreate(uart_port_t p, int16_t inQueueSize, int16_t outQueueSize,
                                UBaseType_t uxPriority, xMemoryPoolHandle xMPool)
{
    // 开辟空间
    xUARTTask_t *newObject = (xUARTTask_t *)malloc(sizeof(xUARTTask_t));
    // socket句柄
    newObject->unum = p;
    sprintf(newObject->send_task_name, "UARTSEND_%d", p);
    sprintf(newObject->rec_task_name, "UARTREC_%d", p);

    // 初始化接收队列
    newObject->rec_queue = xQueueCreate(inQueueSize, sizeof(xMemoryBlockHandle));
    // 初始化发送队列
    newObject->send_queue = xQueueCreate(outQueueSize, sizeof(xMemoryBlockHandle));
    // 任务优先级
    newObject->uxPriority = uxPriority;
    newObject->xMPool = xMPool;
    // 默认回调
    newObject->RecCallback = xUARTRecCallBack;

    newObject->friend = NULL;
    return newObject;
}

void inline xUARTTaskSend(xUARTTaskHandle xUARTTask, const xMemoryBlockHandle xMB)
{
    xQueueSend(xUARTTask->send_queue, &xMB, 0);
}

void xUARTTaskSetRecCallback(xUARTTaskHandle xUARTTask, xUARTRecCallBack_t xUARTRCb)
{
    xUARTTask->RecCallback = xUARTRCb;
}

void xUARTTaskStart(xUARTTaskHandle xUARTTask)
{
    xUARTTask->rec_task_handle = xTaskCreate(uart_rec_task, xUARTTask->rec_task_name,
                                             2048, (void *)xUARTTask, xUARTTask->uxPriority, NULL);
    xUARTTask->send_task_handle = xTaskCreate(uart_send_task, xUARTTask->send_task_name,
                                              2048, (void *)xUARTTask, xUARTTask->uxPriority, NULL);
}

/* UART主要任务，接收数据，然后处理，通过任务实现异步 */
static void uart_rec_task(void *parameter)
{
    ssize_t len;
    xUARTTaskHandle object = (xUARTTaskHandle)parameter;
    while (1)
    {
        xMemoryBlockHandle xMB = xMemoryBlockGet(object->xMPool, 5 / portTICK_RATE_MS);
        if (!xMB)
            continue;

        len = uart_read_bytes(object->unum, xMB->mem, object->xMPool->buf_size, 5 / portTICK_RATE_MS);
        if (len >= object->xMPool->buf_size - 1)
        { // is full?
            xMemoryBlockRelease(object->xMPool, xMB);
            continue;
        }
        else if(len > 0)
        {
            xMB->vaild_size = len;
            object->RecCallback(object, (void *)xMB, object->friend);
        }
        else
        {
            xMemoryBlockRelease(object->xMPool, xMB);
        }
        
    }
    vTaskDelete(NULL);
}

static void uart_send_task(void *parameter)
{
    xUARTTaskHandle object = (xUARTTaskHandle)parameter;
    BaseType_t is_success;
    xMemoryBlockHandle buffer;
    while (1)
    {
        is_success = xQueueReceive(object->send_queue, &buffer, 1000 / portTICK_RATE_MS);
        if (is_success == pdTRUE)
        {
            uart_write_bytes(object->unum, buffer->mem, buffer->vaild_size);
            xMemoryBlockRelease(object->xMPool, buffer);
        }
    }
    vTaskDelete(NULL);
}

void xUARTRecCallBack(xUARTTaskHandle xUARTTask, void *data, void* friend)
{
    // DO NO THING
}