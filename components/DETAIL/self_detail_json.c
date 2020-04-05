#include "self_detail_json.h"

cJSON *device_detail_json;
char device_detail_char[160];
cJSON *json_detail_init()
{
    device_detail_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(device_detail_json, "TYP", 1);
    cJSON_AddStringToObject(device_detail_json, "COMPANY", "ShanghaiShiWei");
    cJSON_AddNumberToObject(device_detail_json, "ID", selfID);
    cJSON_AddNumberToObject(device_detail_json, "KEY", secret_key);
    cJSON_AddNumberToObject(device_detail_json, "SEQ", 0);
    if (cJSON_PrintPreallocated(device_detail_json, device_detail_char, 128, 0))
        ESP_LOGI("[DeviceDetail]", "%s", device_detail_char);
    return device_detail_json;
}

char *get_char_detail()
{
    if (cJSON_PrintPreallocated(device_detail_json, device_detail_char, 128, 0))
    {
        return device_detail_char;
    }
    return (char *)NULL;
}

char *get_char_detail_buff(char *buff, int len)
{
    if (cJSON_PrintPreallocated(device_detail_json, buff, len, 0))
    {
        return device_detail_char;
    }
    return (char *)NULL;
}
