#include "main.h"
#include "link_wifi.h"

#define BUF_SIZE (256)
static int bridge_sock = -1;

const char *TAG = "UART_UDP_BRIDGE";
const char *device_detail = "!#packaheType(A)::company(shiwei)::deviceType(multi)::ID(1)#!";
struct sockaddr_in serviceAddr;
static xSemaphoreHandle count_semaphore;

int  socket_init();
void find_service();
int  wait_service();
void main_loop();
int  conform_service_s1(char *c, int len);
int  conform_service_s2(char *c, int len);
int  close_link();

void udp2uart_task(void *parameters);
void uart2udp_task(void *parameters);
void link_complete_callback(void *parameters);

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    // init semaphore
    count_semaphore = xSemaphoreCreateCounting(2, 0);

    // [block] link to the wifi
    wifi_init_sta();

    uart0_init();
    // task will creat in the mainloop
    main_loop();
}

int socket_init()
{
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(10086);

    bridge_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (bridge_sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return 0;
    }
    ESP_LOGI(TAG, "Socket created");

    int err = bind(bridge_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    if (err < 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket binded");

    return 1;
}

void main_loop()
{
    char rx_buffer[128];
    char addr_str[128];
    static int status = 0;
    while (1)
    {
        if(bridge_sock == -1)
        {
            while(!socket_init())
                vTaskDelay(500/portTICK_RATE_MS);
        }
        while (1)
        {
            struct sockaddr_in sourceAddr;
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(bridge_sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);
            if (len < 0)
            {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            }
            else
            {
                if (status == 0)
                {
                    if (!conform_service_s1(rx_buffer, len))
                    {
                        continue;
                    }
                    // get the service address
                    serviceAddr = sourceAddr;

                    // show service address
                    inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                    rx_buffer[len] = 0;
                    ESP_LOGI(TAG, "Received The Link Require");

                     // send device ID
                    int err = sendto(bridge_sock, device_detail, strlen(device_detail), 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                    if (err < 0)
                    {
                        ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                        break;
                    }
                    ESP_LOGI(TAG, "Sended ACK Waitting For Service Ack");
                    status = 1;
                    continue;
                }
                else if (status == 1)
                {
                    if (!conform_service_s2(rx_buffer, len) || sourceAddr.sin_addr.s_addr != serviceAddr.sin_addr.s_addr)
                    {
                        status = 0;
                        continue;
                    }
                    ESP_LOGI(TAG, "Received The ACK, Start Creating The Task");
                    // confirm complete, create tasks
                    xTaskCreate(udp2uart_task,"udp2uart_task",2048,NULL,5,NULL);
                    xTaskCreate(uart2udp_task,"uart2udp_task",2048,NULL,5,NULL);

                    xSemaphoreTake(count_semaphore, portMAX_DELAY);
                    xSemaphoreTake(count_semaphore, portMAX_DELAY); 
                    ESP_LOGI(TAG, "ERROR Occurred Durning The TASKS");
                    status = 0;
                }
            }
        }
        if (bridge_sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(bridge_sock, 0);
            close(bridge_sock);
        }
    }
}

// TODO
int conform_service_s1(char *c, int len)
{
    return 1;
}

// TODO
int conform_service_s2(char *c, int len)
{
    return 1;
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
            if(tempAddr.sin_addr.s_addr != serviceAddr.sin_addr.s_addr)
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
