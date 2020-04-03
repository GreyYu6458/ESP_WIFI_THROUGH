#include "self_detail_json.h"

cJSON* device_detail_json;
char device_detail_char[64];
cJSON* json_detail_init()
{
    device_detail_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(device_detail_json, "TYP", 1);
    cJSON_AddStringToObject(device_detail_json, "COMPANY", "ShanghaiShiWei");
    cJSON_AddNumberToObject(device_detail_json, "ID", selfID);
    cJSON_AddNumberToObject(device_detail_json, "KEY", secret_key);
    cJSON_AddNumberToObject(device_detail_json, "SEQ", 0);
    return device_detail_json;
}

char* get_char_detail()
{  
    if(cJSON_PrintPreallocated(device_detail_json, device_detail_char,64,0))
    {
        return device_detail_char;
    }
    return (char*)NULL;
}

