#include "main.h"
#include "link_wifi.h"

void app_main()
{
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
    int scan_socket = -1;
    while ((scan_socket = socket_init(10086)) == -1)
        vTaskDelay(500 / portTICK_RATE_MS);

    /* 验证任务 */
    xMemoryPoolHandle xMPH = xMemoryPoolCreate(2, 128);
    xUDPTaskHandle udpScanTask = xUDPTaskCreate(scan_socket, NULL, 2, 2, 4, xMPH);
    xAuthenticateTaskHandle authenticateTask = xAuthenticateTaskCreate(udpScanTask);
    xAuthenticateTaskStart(authenticateTask); // 开始任务
}
