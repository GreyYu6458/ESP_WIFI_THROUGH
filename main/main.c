#include "main.h"
#include "link_wifi.h"
#include <time.h>

/* 自身ID */
static const u8_t   selfID = 0;
/* 本机描述信息 */
char*               device_detail_char;
/* 被扫描用socket */
static int          scan_socket = -1;
/* uart2udp bridge 用socket */
static int          bridge_socket = -1;
/* 调试用 */
const char*         TAG = "UART_UDP_BRIDGE";
/* 链接的密钥 */
const int32_t       confirm_key = 49107652;
/* 分配的端口 */
int32_t             assigned_port;
/* 本机描述JSON */
cJSON*              device_detail;
/* 通讯用随机序列 */
int32_t             random_seq;
/* 服务器地址，在第一次被扫描时初始化 */
struct sockaddr_in  serviceAddr;
/* 桥初始化互斥量 */
static xSemaphoreHandle     bridge_socket_mutex;
static xSemaphoreHandle     mission_count_semaphore;

/* UDP 互斥量 */
static xSemaphoreHandle     UDP_send_mutex;

/* 用于异步传输的队列 */
static xQueueHandle         udp2uart_queue_handle;
static xQueueHandle         uart2udp_queue_handle;

/* 套接字信号量 */
int socket_init(uint16_t port);

/* 扫描回复任务 */
void scan_task(void *parameters);

/* 接收任务 */
void udp_recv_task(void* parameters);
void uart_recv_task(void* parameters);

/* 发送任务 */
void uart_send_task(void* parameter);
void udp_send_task(void* parameter);

/* udp2uart任务 */
void udp2uart_task(void *parameters);

/* uart2udp任务 */
void uart2udp_task(void *parameters);

void app_main()
{
    conform_event_init(on_con_1, on_con_2, on_fai);

    /* 生成本机信息json对象 */
    device_detail = cJSON_CreateObject();
    cJSON_AddNumberToObject(device_detail, "TYP", 1);
    cJSON_AddStringToObject(device_detail, "COMPANY", "ShanghaiShiWei");
    cJSON_AddNumberToObject(device_detail, "ID", 0);
    cJSON_AddNumberToObject(device_detail, "KEY", confirm_key);
    cJSON_AddNumberToObject(device_detail, "SEQ",0);

    /* 初始化队列 */
    uart2udp_queue_handle = xQueueCreate(10, sizeof(inner_data*));
    udp2uart_queue_handle = xQueueCreate(10, sizeof(inner_data*));

    /* 检测flash是否运行正常 */
    ESP_ERROR_CHECK(nvs_flash_init());
    
    /* 初始化信号量 */
    bridge_socket_mutex     = xSemaphoreCreateMutex();
    UDP_send_mutex          = xSemaphoreCreateMutex();
    mission_count_semaphore = xSemaphoreCreateCounting(2,0);

    /* 连接到wifi */
    wifi_init_sta();

    /* 初始化串口 */
    uart0_init();
    // task will creat in the mainloop
    if (scan_socket == -1)
    {
        while ((scan_socket = socket_init(10086)) == -1)
            vTaskDelay(500 / portTICK_RATE_MS);
    }
    if (bridge_socket == -1)
    {
        while ((bridge_socket = socket_init(60001 + selfID)) == -1)
            vTaskDelay(500 / portTICK_RATE_MS);
    }

    xTaskCreate(udp2uart_task,  "udp2uart_task",    2048, (void*)NULL, 5, NULL);
    xTaskCreate(uart2udp_task,  "uart2udp_task",    2048, (void*)NULL, 5, NULL);
    xTaskCreate(scan_task,      "scan_task",        2048, (void*)NULL, 4, NULL);
}

int socket_init(uint16_t port)
{
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        return 0;
    }

    int err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));

    return sock;
}

#define _SCAN_TASK_BUFF_SIZE (300)
void scan_task(void *parameters)
{
    int32_t seq;
    char rx_buffer[_SCAN_TASK_BUFF_SIZE];
    // char addr_str[128];
    static int status = 0;
    while (1)
    {
        if (scan_socket == -1)
        {
            while ((scan_socket = socket_init(10086)) == -1)
                vTaskDelay(2000 / portTICK_RATE_MS);
        }
        while (1)
        {
            struct sockaddr_in sourceAddr;
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(scan_socket, rx_buffer, sizeof(rx_buffer) - 1, 0, 
                                (struct sockaddr *)&sourceAddr, &socklen);
            // if (len < 0)
            // {
            //     // ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            // }
            if (len >= _SCAN_TASK_BUFF_SIZE - 1)
            {// is full?
                continue;
            }
            else
            {
                if (status == 0)
                {
                    // process as a string
                    rx_buffer[len] = 0;
                    conform_service(rx_buffer, status);
                    if (!seq)
                    {
                        continue;
                    }
                    // get the service address
                    serviceAddr = sourceAddr;
                    cJSON_ReplaceItemInObject(device_detail, "SEQ", cJSON_CreateNumber(seq + 1));
                    device_detail_char = cJSON_Print(device_detail);
                    ESP_LOGI("%s",device_detail_char);
                    int err = sendto(scan_socket, device_detail_char, strlen(device_detail_char), 0, 
                                    (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                    free(device_detail_char);
                    if (err < 0)
                    {
                        break;
                    }
                    status = 1;
                    continue;
                }
                else if (status == 1)
                {
                    // confirm address
                    conform_service(rx_buffer, status);
                    if (!seq || sourceAddr.sin_addr.s_addr != serviceAddr.sin_addr.s_addr)
                    {
                        status = 0;
                        continue;
                    }
                    
                    serviceAddr.sin_port = htons(assigned_port);

                    cJSON_ReplaceItemInObject(device_detail, "SEQ", cJSON_CreateNumber(seq + 1));
                    device_detail_char = cJSON_Print(device_detail);
                    ESP_LOGI("%s",device_detail_char);
                    int err = sendto(scan_socket, device_detail_char, strlen(device_detail_char), 0, 
                                            (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                    free(device_detail_char);
                    if (err < 0)
                    {
                        break;
                    }
                    // confirm complete, start mission
                    xSemaphoreGive(mission_count_semaphore);
                    xSemaphoreGive(mission_count_semaphore);
                    status = 0;
                }
            }
        }
        if (scan_socket != -1)
        {
            shutdown(scan_socket, 0);
            close(scan_socket);
            scan_socket = -1;
        }
    }
}



#define _BRIDGE_UDP_BUFF_SIZE (128)
void udp2uart_task(void *parameters)
{
    char rx_buffer[_BRIDGE_UDP_BUFF_SIZE];
    // 发送方地址会暂时存在这
    struct sockaddr_in tempAddr;
    socklen_t socklen = sizeof(tempAddr);
    while (1)
    {
        xSemaphoreTake(bridge_socket_mutex, portMAX_DELAY);
        if (bridge_socket == -1)
        {
            while (!(bridge_socket = socket_init(60001 + selfID)))
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        xSemaphoreGive(bridge_socket_mutex);
        xSemaphoreTake(mission_count_semaphore,portMAX_DELAY);
        while (1)
        {
            int len = recvfrom(bridge_socket, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&tempAddr, &socklen);
            // 接收出错
            if (len < 0)
            {
                break;
            }
            // 收到UDP数据,转发数据
            uart_write_bytes(UART_NUM_0, rx_buffer, len);
        }
        if (bridge_socket != -1)
        {
            shutdown(bridge_socket, 0);
            close(bridge_socket);
            bridge_socket = -1;
        }
        xSemaphoreGive(mission_count_semaphore);
    }
    vTaskDelete(NULL);
}

#define BUF_SIZE (64)
void uart2udp_task(void *parameters)
{
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    while (1)
    {
        // sock
        xSemaphoreTake(bridge_socket_mutex, portMAX_DELAY);
        if (bridge_socket == -1)
        {
            while (!(bridge_socket = socket_init(60001 + selfID)))
                vTaskDelay(500 / portTICK_RATE_MS);
        }
        xSemaphoreGive(bridge_socket_mutex);
        xSemaphoreTake(mission_count_semaphore,portMAX_DELAY);
        while (1)
        {
            // 从UART端口读取数据
            int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 10 / portTICK_RATE_MS);
            if (len <= 0)
            {
                continue;
            }
            int err = sendto(bridge_socket, data, len, 0, (struct sockaddr *)&serviceAddr, sizeof(serviceAddr));
            if (err < 0)
            {
                break;
            }
        }
        if (bridge_socket != -1)
        {
            shutdown(bridge_socket, 0);
            close(bridge_socket);
            bridge_socket = -1;
        }
        xSemaphoreGive(mission_count_semaphore);
    }
    free(data);
    vTaskDelete(NULL);
}

void udp_recv_task(void* parameters)
{
    char rx_buffer[256];
    // 发送方地址会暂时存在这
    struct sockaddr_in tempAddr;
    socklen_t socklen = sizeof(tempAddr);
    while (1)
    {
        xSemaphoreTake(bridge_socket_mutex, portMAX_DELAY);
        if (bridge_socket == -1)
        {
            while (!(bridge_socket = socket_init(60001 + selfID)))
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        xSemaphoreGive(bridge_socket_mutex);
        xSemaphoreTake(mission_count_semaphore,portMAX_DELAY);
        while (1)
        {
            int len = recvfrom(bridge_socket, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&tempAddr, &socklen);
            // 接收出错
            if (len < 0)
            {
                break;
            }
            // 收到UDP数据,转发数据
            uart_write_bytes(UART_NUM_0, rx_buffer, len);
        }
        if (bridge_socket != -1)
        {
            shutdown(bridge_socket, 0);
            close(bridge_socket);
            bridge_socket = -1;
        }
        xSemaphoreGive(mission_count_semaphore);
    }
    vTaskDelete(NULL);
}

