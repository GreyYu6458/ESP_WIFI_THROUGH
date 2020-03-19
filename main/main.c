#include "main.h"
#include "link_wifi.h"
#include "cJSON.h"
#include <time.h>

#define BUF_SIZE (256)
static int bridge_sock = -1;

const char *TAG = "UART_UDP_BRIDGE";
int32_t confirm_key = 49107652;
int32_t assigned_port;
cJSON *device_detail;
const u8_t selfID = 0;
char *device_detail_char;

int32_t random_seq;
struct sockaddr_in serviceAddr;
static xSemaphoreHandle count_semaphore;

int socket_init(uint16_t port);
void find_service();
int wait_service();
void main_loop();
int32_t conform_service(char *c, int sc); // shake count
int close_link();

void udp2uart_task(void *parameters);
void uart2udp_task(void *parameters);
void link_complete_callback(void *parameters);

void app_main()
{
    // 生成本机信息json对象
    device_detail = cJSON_CreateObject();
    cJSON_AddNumberToObject(device_detail, "TYP", 1);
    cJSON_AddStringToObject(device_detail, "COMPANY", "ShanghaiShiWei");
    cJSON_AddNumberToObject(device_detail, "ID", 0);
    cJSON_AddNumberToObject(device_detail, "KEY", confirm_key);

    ESP_ERROR_CHECK(nvs_flash_init());
    // ESP_LOGI(TAG, "// ESP_WIFI_MODE_STA");
    // init semaphore
    count_semaphore = xSemaphoreCreateCounting(2, 0);

    // [block] link to the wifi
    wifi_init_sta();

    uart0_init();
    // task will creat in the mainloop
    main_loop();
}

int socket_init(uint16_t port)
{
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);

    bridge_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (bridge_sock < 0)
    {
        // ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return 0;
    }
    // ESP_LOGI(TAG, "Socket created");

    int err = bind(bridge_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    if (err < 0)
    {
        // ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    // ESP_LOGI(TAG, "Socket binded");
    return bridge_sock;
}

void main_loop()
{
    int32_t seq;
    char rx_buffer[256];
    // char addr_str[128];
    static int status = 0;
    while (1)
    {
        if (bridge_sock == -1)
        {
            while (!socket_init(10086))
                vTaskDelay(500 / portTICK_RATE_MS);
        }
        while (1)
        {
            struct sockaddr_in sourceAddr;
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(bridge_sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);
            if (len < 0)
            {
                // ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            }
            else if (len >= 255)
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
                    ESP_LOGI(TAG,"R1:%s",rx_buffer);
                    if (!seq)
                    {
                        continue;
                    }
                    // get the service address
                    serviceAddr = sourceAddr;

                    // ESP_LOGI(TAG, "Received The Link Require");

                    cJSON_AddNumberToObject(device_detail, "SEQ", seq + 1);
                    device_detail_char = cJSON_Print(device_detail);

                    int err = sendto(bridge_sock, device_detail_char, strlen(device_detail_char), 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                    ESP_LOGI(TAG,"S:%s",device_detail_char);
                    cJSON_DeleteItemFromObject(device_detail,"SEQ");
                    free(device_detail_char);
                    if (err < 0)
                    {
                        // ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                        break;
                    }
                    // ESP_LOGI(TAG, "Sended ACK Waitting For Service Ack");
                    status = 1;
                    continue;
                }
                else if (status == 1)
                {
                    // confirm address
                    ESP_LOGI(TAG,"R2:%s",rx_buffer);
                    seq = conform_service(rx_buffer, status);
                    if (!seq || sourceAddr.sin_addr.s_addr != serviceAddr.sin_addr.s_addr)
                    {
                        status = 0;
                        continue;
                    }
                    cJSON_AddNumberToObject(device_detail, "SEQ", seq + 1);
                    device_detail_char = cJSON_Print(device_detail);

                    int err = sendto(bridge_sock, device_detail_char, strlen(device_detail_char), 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                    
                    ESP_LOGI(TAG,"S:%s",device_detail_char);
                    serviceAddr.sin_port = htons(assigned_port);
                    cJSON_DeleteItemFromObject(device_detail,"SEQ");
                    free(device_detail);
                    if (err < 0)
                    {
                        // ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                        break;
                    }


                    ESP_LOGI(TAG,"rebuild socket communication");
                    // confirm complete, create tasks

                    shutdown(bridge_sock, 0);
                    close(bridge_sock);
                    if(!socket_init(60001+selfID))
                    {
                        ESP_LOGI(TAG,"rebind the port");
                        break;
                    }

                    xTaskCreate(udp2uart_task, "udp2uart_task", 2048, NULL, 5, NULL);
                    xTaskCreate(uart2udp_task, "uart2udp_task", 2048, NULL, 5, NULL);

                    xSemaphoreTake(count_semaphore, portMAX_DELAY);
                    xSemaphoreTake(count_semaphore, portMAX_DELAY);
                    // ESP_LOGI(TAG, "ERROR Occurred Durning The TASKS");
                    status = 0;
                }
            }
        }
        if (bridge_sock != -1)
        {
            // ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(bridge_sock, 0);
            close(bridge_sock);
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
    
    ESP_LOGI(TAG,"FLAG");
    int32_t TYP = cJSON_GetObjectItem(input, "TYP")->valueint;
    int32_t SEQ = cJSON_GetObjectItem(input, "SEQ")->valueint;
    int32_t KEY = cJSON_GetObjectItem(input, "KEY")->valueint;
    cJSON_Delete(input);
    switch (sc)
    {
    case 0:
        if (TYP == 0 && KEY==confirm_key)
        {
            ESP_LOGI(TAG,"FLAG2");
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
    char rx_buffer[512];
    // 发送方地址会暂时存在这
    struct sockaddr_in tempAddr;
    socklen_t socklen = sizeof(tempAddr);
    while (1)
    {
        if (bridge_sock == -1)
        {
            break;
        }
        while (1)
        {
            // 这里也会将发送者的地址存入serverAddr
            int len = recvfrom(bridge_sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&tempAddr, &socklen);
            // 接收出错
            if (len < 0)
            {
                break;
            }
            if (tempAddr.sin_addr.s_addr != serviceAddr.sin_addr.s_addr)
            {
                continue;
            }
            // 收到UDP数据,转发数据
            uart_write_bytes(UART_NUM_0, rx_buffer, len);
            rx_buffer[len] = 0;
        }
    }
    xSemaphoreGive(count_semaphore);
    vTaskDelete(NULL);
}

void uart2udp_task(void *parameters)
{
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    while (1)
    {
        // sock未创建
        if (bridge_sock == -1)
        {
            break;
        }
        while (1)
        {
            // 从UART端口读取数据
            int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 10 / portTICK_RATE_MS);
            if (len <= 0)
            {
                continue;
            }
            int err = sendto(bridge_sock, data, len, 0, (struct sockaddr *)&serviceAddr, sizeof(serviceAddr));
            if (err < 0)
            {
                break;
            }
        }
    }
    free(data);
    xSemaphoreGive(count_semaphore);
    vTaskDelete(NULL);
}
