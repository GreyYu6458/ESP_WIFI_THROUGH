#include "main.h"
#include "link_wifi.h"

xSemaphoreHandle bridgeMutex;

void app_main()
{
    bridgeMutex = xSemaphoreCreateBinary();
    /* 检测flash是否运行正常 */
    ESP_ERROR_CHECK(nvs_flash_init());
    /* 初始化串口 UART0 921600 8 */
    uart_init(UART_NUM_0, 921600, UART_DATA_8_BITS, UART_PARITY_DISABLE,
              UART_STOP_BITS_1, UART_HW_FLOWCTRL_DISABLE);
    /* 连接到wifi */
    wifi_init_sta();
    /* 构建本机信息 */
    json_detail_init();
    // task will creat in the mainloop
    int scan_socket = -1, bridge_socket = -1;
    while ((scan_socket = socket_init(10086)) == -1)
        vTaskDelay(500 / portTICK_RATE_MS);
    while ((bridge_socket = socket_init(60001 + selfID)) == -1)
        vTaskDelay(500 / portTICK_RATE_MS);
    /* 验证任务 */
    xMemoryPoolHandle xMPH = xMemoryPoolCreate(2, 128);
    xUDPTaskHandle udpScanTask = xUDPTaskCreate(scan_socket, NULL, 2, 2, 4, xMPH);
    xAuthenticateTaskHandle authenticateTask = xAuthenticateTaskCreate(udpScanTask);
    xAuthenticateTaskStart(authenticateTask); // 开始任务


    xSemaphoreTake(bridgeMutex, portMAX_DELAY);
    /* 传输任务 */
    xMemoryPoolHandle xMPBH = xMemoryPoolCreate(20, 1024);
    xUDPTaskHandle udpBridgeTask = xUDPTaskCreate(bridge_socket, NULL, 10, 10, 5, xMPBH);
    xUARTTaskHandle uartBridgeTask = xUARTTaskCreate(UART_NUM_0, 10, 10, 5, xMPBH);

    xBridgeTaskHandle bridgeTask = xBridgeTaskCreate(uartBridgeTask, udpBridgeTask);

    {
        char addr_str[128];
        inet_ntoa_r(serviceAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(TAG, "START ON:%s:%d", addr_str, serviceAddr.sin_port);
    }
    
    xUDPTaskSetRemote(udpBridgeTask, &serviceAddr);
    xBridgeTaskStart(bridgeTask);
}
