#include "udp_task.h"

static void udp_rec_task(void *parameter);
static void udp_send_task(void *parameter);
static void xUDPRecCallBack(xUDPTaskHandle xUDPTask, void * data, struct sockaddr_in * to);

xUDPTaskHandle xUDPTaskCreate(int sock, struct sockaddr *to, int16_t inQueueSize,
                              int16_t outQueueSize, UBaseType_t uxPriority, xMemoryPoolHandle xMPool)
{
    // 开辟空间
    xUDPTask_t *newObject = (xUDPTask_t *)malloc(sizeof(xUDPTask_t));
    // socket句柄
    newObject->hsocket = sock;
    sprintf(newObject->send_task_name, "UDPSEND_%d", sock);
    sprintf(newObject->rec_task_name, "UDPREC_%d", sock);
    // 初始化接收队列
    newObject->udp_rec_queue = xQueueCreate(inQueueSize, sizeof(xMemoryBlockHandle));
    // 初始化发送队列
    newObject->udp_send_queue = xQueueCreate(outQueueSize, sizeof(xMemoryBlockHandle));
    // 任务优先级
    newObject->uxPriority = uxPriority;
    newObject->xMPool = xMPool;
    // 默认回调
    newObject->xUDPRecCallback = xUDPRecCallBack;

    return newObject;
}

void xUDPTaskStart(xUDPTaskHandle xUDPTask)
{
    xUDPTask->rec_task_handle = xTaskCreate(udp_rec_task, xUDPTask->rec_task_name, 
                                            2048, (void *)xUDPTask, xUDPTask->xMPool, NULL);
    xUDPTask->send_task_handle = xTaskCreate(udp_send_task, xUDPTask->send_task_name, 
                                            2048, (void *)xUDPTask, xUDPTask->xMPool, NULL);
}

void xUDPTaskSetRemote(xUDPTaskHandle xUDPTask, struct sockaddr_in *to)
{
    xUDPTask->to = *to;
}

void xUDPTaskSend(xUDPTaskHandle xUDPTask, const xMemoryBlockHandle xMB)
{
    xQueueSend(xUDPTask->udp_send_queue, &xMB, 0);
}

void xUDPTaskSetRecCallback(xUDPTaskHandle xUDPTask, xUDPRecCallBack_t xUDPRCb)
{
    xUDPTask->xUDPRecCallback = xUDPRCb;
}

/* UDP主要任务，接收数据，然后处理，通过任务实现异步 */
static void udp_rec_task(void *parameter)
{
    ssize_t len;
    struct sockaddr_in sourceAddr;
    socklen_t socklen = sizeof(sourceAddr);
    xUDPTaskHandle object = (xUDPTaskHandle)parameter;
    while (1)
    {
        xMemoryBlockHandle xMB = xMemoryBlockGet(object->xMPool);
        if(!xMB) continue;
        len = recvfrom(object->hsocket, xMB->mem, object->xMPool->buf_size, 0, (struct sockaddr *)&sourceAddr, &socklen);
        if (len >= object->xMPool->buf_size - 1)
        { // is full?
            xMemoryBlockRelease(object->xMPool, xMB);
            continue;
        }
        xMB->vaild_size = len;
        object->xUDPRecCallback(object, (void *)xMB, &sourceAddr); // 重写这个函数
        // 不要在此释放内存，考虑到rec_callback可能是异步
    }
    vTaskDelete(NULL);
}

static void udp_send_task(void *parameter)
{
    xUDPTaskHandle object = (xUDPTaskHandle)parameter;
    BaseType_t is_success;
    xMemoryBlockHandle buffer;
    while (1)
    {
        is_success = xQueueReceive(object->udp_send_queue, &buffer, 1000 / portTICK_RATE_MS);
        if (is_success == pdTRUE)
        {
            sendto(object->hsocket, buffer->mem, buffer->vaild_size, 0, (struct sockaddr *)&object->to, sizeof(object->to));
            xMemoryBlockRelease(object->xMPool, buffer);
        }
    }
    vTaskDelete(NULL);
}


static void xUDPRecCallBack(xUDPTaskHandle xUDPTask, void * data, struct sockaddr_in * to)
{
    // DO NOTHING
} 