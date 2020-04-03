#include "conform.h"

void conform_service(char *c, struct sockaddr_in* sourceAddr)
{
    static int sc = 0;
    cJSON *input = cJSON_Parse((const char *)c);
    if( !input                            ||!cJSON_HasObjectItem(input, "TYP")||
        !cJSON_HasObjectItem(input, "SEQ")||!cJSON_HasObjectItem(input, "KEY"))
        goto fail;
    static int32_t seq;
    int32_t TYP = cJSON_GetObjectItem(input, "TYP")->valueint;
    int32_t KEY = cJSON_GetObjectItem(input, "KEY")->valueint;

    switch (sc)
    {
    case LINK_FIRST:
        if (TYP == 0 && KEY==confirm_key)
        {
            if(!on_con_1(input, sourceAddr))
            {
                goto fail;
            }
            sc++;
        }
        break;
    case LINK_SECOND:
        if (TYP == 0 && KEY == confirm_key)
        {
            if(!on_con_2(input,sourceAddr))
            {
                goto fail;
            } 
            sc++;
        }
        break;
    case LINKING:
        break;
    }
    cJSON_Delete(input);
    return;

    fail:
    sc = 0;
    on_failed(input, sourceAddr);
    cJSON_Delete(input);
}

inline void udp_conform_callback(void* data, struct sockaddr_in* sourceAddr,UDPObject* this)
{
    transport_data* d = (transport_data*)data;
    d->data[d->len] = 0;
    conform_service(d->data, sourceAddr);
}