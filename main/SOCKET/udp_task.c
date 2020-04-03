#include "udp_task.h"

// in_queue的长度
#define _IQ_LEN (10)
// out_queue的长度
#define _OQ_LEN (10)
// in 传输数据类型的大小
#define _ITD_SIZE sizeof(indefiniteData)
// in 传输数据类型的大小
#define _OTD_SIZE sizeof(indefiniteData)
// UDP RECVICE BUFF SIZE
#define _UDP_RECVICE_BUFF_SIZE (300)

#define _UDP_TASK_PROIORITY 5

static void udp_task_start(UDPObject *this);
static void udp_rec_task(void *parameter);
static void udp_send_task(void *parameter);

static void UDPObject_udp_send(const indefiniteData *data, UDPObject *this);
static void UDPObject_default_rec_callback(void *data, struct sockaddr_in *sourceAddr, UDPObject *this);
static void UDPObject_send(const indefiniteData *data, struct sockaddr *to, UDPObject *this);

UDPObject *UDPObject_Consturct(int s, const char *task_name, struct sockaddr *to)
{
    // 开辟空间
    UDPObject *newObject = (UDPObject *)malloc(sizeof(UDPObject));
    newObject->this = newObject;

    // socket句柄
    newObject->hsocket = s;
    newObject->rec_task_name = (char *)malloc(sizeof(char) * 15);
    newObject->send_task_name = (char *)malloc(sizeof(char) * 15);
    sprintf(newObject->send_task_name, "UDPSEND_%d", s);
    sprintf(newObject->rec_task_name, "UDPREC_%d", s);

    // 初始化接收队列
    newObject->udp_rec_queue = xQueueCreate(_IQ_LEN, _ITD_SIZE);
    // 初始化发送队列
    newObject->udp_send_queue = xQueueCreate(_OQ_LEN, _OTD_SIZE);
    // 接收方地址
    newObject->to = to;

    // 接收到数据后的回调
    newObject->rec_callback = UDPObject_default_rec_callback;
    // 发送
    newObject->send = UDPObject_udp_send;
    // 开始任务
    newObject->rec_task_start = udp_task_start;
    return newObject;
}

/* 发送函数 */
static inline void UDPObject_udp_send(const indefiniteData *data, UDPObject *this)
{
    xQueueSend(this->udp_send_queue, data, 0);
}

/* 开始任务 */
static void udp_task_start(UDPObject *this)
{
    this->rec_task_handle = (udp_rec_task, this->rec_task_name, 1024, (void *)this, _UDP_TASK_PROIORITY, NULL);
    this->send_task_handle = (udp_send_task, this->send_task_name, 1024, (void *)this, _UDP_TASK_PROIORITY, NULL);
}

/* UDP主要任务，接收数据，然后处理，通过任务实现异步 */
static void udp_rec_task(void *parameter)
{
    ssize_t len;
    struct sockaddr_in sourceAddr;
    socklen_t socklen = sizeof(sourceAddr);
    UDPObject *object = (UDPObject *)parameter;
    while (1)
    {
        char* rx_buffer = (char*)malloc(_UDP_RECVICE_BUFF_SIZE);
        len = recvfrom(object->hsocket, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);
        if (len >= _UDP_RECVICE_BUFF_SIZE - 1)
        { // is full?
            continue;
        }
        object->to = &sourceAddr;
        indefiniteData data ={.len = len,.data = rx_buffer};
        object->rec_callback((void *)&data, &sourceAddr, object); // 重写这个函数
        
        // 不要在此释放内存，考虑到rec_callback可能是异步
    }
    vTaskDelete(NULL);
}

static void udp_send_task(void *parameter)
{
    UDPObject *object = (UDPObject *)parameter;
    BaseType_t is_success;
    indefiniteData buffer;
    while (1)
    {
        is_success = xQueueReceive(object->udp_send_queue, &buffer, 5 / portTICK_RATE_MS);
        if (is_success == pdTRUE)
        {
            sendto(object->hsocket, buffer.data, buffer.len, 0, object->to, sizeof(object->to));
            free(buffer.data);
        }
    }
    vTaskDelete(NULL);
}

static void UDPObject_default_rec_callback(void *data, struct sockaddr_in *sourceAddr, UDPObject *this)
{
    // do nothing
}

void UDPObject_Delete(UDPObject *ob)
{
    vTaskDelete(ob->rec_task_handle);
    vTaskDelete(ob->send_task_handle);
    free(ob->rec_task_name);
    free(ob->send_task_name);
    free(ob);
}