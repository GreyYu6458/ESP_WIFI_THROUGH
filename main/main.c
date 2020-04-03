#include "main.h"
#include "link_wifi.h"

void app_main()
{
    /* 检测flash是否运行正常 */
    ESP_ERROR_CHECK(nvs_flash_init());
    /* 连接到wifi */
    wifi_init_sta();
    /* 构建本机信息 */
    create_json_detail();
    /* 初始化串口 */
    uart0_init();
    // task will creat in the mainloop
    int scan_socket = -1;
    if (scan_socket == -1)
    {
        while ((scan_socket = socket_init(10086)) == -1)
            vTaskDelay(500 / portTICK_RATE_MS);
    }

    UDPObject* scan_udp_adapter = UDPObject_Consturct(scan_socket,NULL);
    AuthenticateTask* authenticate_task = AuthenticateTask_Consturct(scan_udp_adapter);
    authenticate_task->authenticate_start(authenticate_task);
}
