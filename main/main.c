#include "main.h"
#include "link_wifi.h"
#include "cJSON.h"
#include <time.h>

#define BUF_SIZE (64)

const char *TAG = "UART_UDP_BRIDGE";
int32_t confirm_key = 49107652;
int32_t assigned_port;
cJSON *device_detail;
const u8_t selfID = 0;
char *device_detail_char;

int32_t random_seq;
struct sockaddr_in serviceAddr;
static xSemaphoreHandle bridge_socket_mutex;
static xSemaphoreHandle mission_count_semaphore;

int socket_init(uint16_t port);

int32_t conform_service(char *c, int sc); // shake count

static int scan_socket = -1 ,bridge_socket = -1;
void scan_task(void *parameters);
void udp2uart_task(void *parameters);
void uart2udp_task(void *parameters);


void app_main()
{
    // 生成本机信息json对象
    device_detail = cJSON_CreateObject();
    cJSON_AddNumberToObject(device_detail, "TYP", 1);
    cJSON_AddStringToObject(device_detail, "COMPANY", "ShanghaiShiWei");
    cJSON_AddNumberToObject(device_detail, "ID", 0);
    cJSON_AddNumberToObject(device_detail, "KEY", confirm_key);

    ESP_ERROR_CHECK(nvs_flash_init());
    // init semaphore
    bridge_socket_mutex     = xSemaphoreCreateMutex();
    mission_count_semaphore = xSemaphoreCreateCounting(2,0);

    // [block] link to the wifi
    wifi_init_sta();

    // 服务器地址默认指向网关


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

void scan_task(void *parameters)
{
    int32_t seq;
    char rx_buffer[256];
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
            if (len >= 255)
            {// is full?
                continue;
            }
            else
            {
                if (status == 0)
                {
                    // process as a string
                    rx_buffer[len] = 0;
                    seq = conform_service(rx_buffer, status);
                    if (!seq)
                    {
                        continue;
                    }
                    // get the service address
                    serviceAddr = sourceAddr;

                    cJSON_AddNumberToObject(device_detail, "SEQ", seq + 1);
                    device_detail_char = cJSON_Print(device_detail);

                    int err = sendto(scan_socket, device_detail_char, strlen(device_detail_char), 0, 
                                           (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                    cJSON_DeleteItemFromObject(device_detail,"SEQ");
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
                    seq = conform_service(rx_buffer, status);
                    if (!seq || sourceAddr.sin_addr.s_addr != serviceAddr.sin_addr.s_addr)
                    {
                        status = 0;
                        continue;
                    }
                    
                    serviceAddr.sin_port = htons(assigned_port);

                    cJSON_AddNumberToObject(device_detail, "SEQ", seq + 1);
                    device_detail_char = cJSON_Print(device_detail);

                    int err = sendto(scan_socket, device_detail_char, strlen(device_detail_char), 0, 
                                            (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                    
                    
                    cJSON_DeleteItemFromObject(device_detail,"SEQ");
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

/*
    message format:
    TYP
    SEQ
    KEY
*/
int32_t conform_service(char *c, int sc)
{
    static int32_t seq;
    cJSON *input = cJSON_Parse((const char *)c);

    if(!input) return 0;
    if(!cJSON_HasObjectItem(input, "TYP")) return 0;
    if(!cJSON_HasObjectItem(input, "SEQ")) return 0;
    if(!cJSON_HasObjectItem(input, "KEY")) return 0;
    
    int32_t TYP = cJSON_GetObjectItem(input, "TYP")->valueint;
    int32_t SEQ = cJSON_GetObjectItem(input, "SEQ")->valueint;
    int32_t KEY = cJSON_GetObjectItem(input, "KEY")->valueint;
    cJSON_Delete(input);
    switch (sc)
    {
    case 0:
        if (TYP == 0 && KEY==confirm_key)
        {
            seq = SEQ;
            return SEQ;
        }
    case 1:
        if (TYP == 0 && SEQ == seq + 2 && KEY == confirm_key)
        {
            if(!cJSON_HasObjectItem(input, "PORT")) return 0;
            else assigned_port = cJSON_GetObjectItem(input,"PORT")->valueint;
            return SEQ;
        }
    }
    return 0;
}

void udp2uart_task(void *parameters)
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
