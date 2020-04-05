#include "uart_task.h"

// in_queue的长度
#define _IQ_LEN (10)
// out_queue的长度
#define _OQ_LEN (10)
// in 传输数据类型的大小
#define _ITD_SIZE sizeof(indefiniteData)
// in 传输数据类型的大小
#define _OTD_SIZE sizeof(indefiniteData)
// UDP RECVICE BUFF SIZE
#define _UART_RECVICE_BUFF_SIZE (300)

#define _UDP_TASK_PROIORITY 5

/*
    uart_config_t uart_config = {
        .baud_rate = UART0_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, UART0_BUF_SIZE * 2, 0, 0, NULL, 0);
*/

static void uart_rec_task(void *parameter);
static void uart_send_task(void *parameter);

static void UARTObject_rec_task_start(struct UARTObject *this);
static void UARTObject_send(const indefiniteData *data, struct UARTObject *this);
static void UARTObject_rec_callback(void *data, struct UARTObject *this);

UARTObject *UARTObject_Construct(uart_port_t u, uart_config_t uart_config, memory_pool *mpool)
{
    UARTObject *newObject = (UARTObject *)malloc(sizeof(UARTObject));
    newObject->this = newObject;
    newObject->huart = u;

    sprintf(newObject->send_task_name, "UARTSEND_%d", u);
    sprintf(newObject->rec_task_name, "UATYREC_%d", u);

    newObject->mpool = mpool;
    // 初始化接收队列
    newObject->rec_queue = xQueueCreate(_IQ_LEN, _ITD_SIZE);
    // 初始化发送队列
    newObject->send_queue = xQueueCreate(_OQ_LEN, _OTD_SIZE);

    newObject->rec_callback = UARTObject_rec_callback;
    newObject->send = UARTObject_send;
    newObject->rec_task_start = UARTObject_rec_task_start;
    return newObject;
}

/* 发送函数 */
static void UARTObject_send(const indefiniteData *data, struct UARTObject *this)
{
    xQueueSend(this->send_queue, data, 0);
}

/* 开始任务 */
static void udp_task_start(struct UARTObject *this)
{
    this->rec_task_handle = xTaskCreate(uart_rec_task, this->rec_task_name, 2048, (void *)this, _UDP_TASK_PROIORITY, NULL);
    this->send_task_handle = xTaskCreate(uart_send_task, this->send_task_name, 2048, (void *)this, _UDP_TASK_PROIORITY, NULL);
}

static void udp_rec_task(void *parameter)
{
    ssize_t len;
    UARTObject *object = (UARTObject *)parameter;
    while (1)
    {
        memory_block *mblock = get_memory_block(object->mpool);
        int len = uart_read_bytes(object->huart, (uint8_t*)mblock->memory->data, mblock->size, 10 / portTICK_PERIOD_MS);
        if (len >= mblock->size - 1)
        { // is full?
            continue;
        }
        mblock->memory->len = len;
        object->rec_callback((void *)mblock, object); // 重写这个函数
        // 不要在此释放内存，考虑到rec_callback可能是异步
    }
    vTaskDelete(NULL);
}

static void udp_send_task(void *parameter)
{
    UARTObject *object = (UARTObject *)parameter;
    BaseType_t is_success;
    indefiniteData buffer;
    while (1)
    {
        is_success = xQueueReceive(object->send_queue, &buffer, 10 / portTICK_RATE_MS);
        if (is_success == pdTRUE)
        {
            uart_write_bytes(object->huart, buffer.data, buffer.len);
            free(buffer.data);
        }
    }
    vTaskDelete(NULL);
}

void UARTObject_Delete(UARTObject *ob)
{
    vTaskDelete(ob->rec_task_handle);
    vTaskDelete(ob->send_task_handle);
    free(ob);
}