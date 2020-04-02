#include "conform.h"

static char is_init = false;
static CallBack on_conformed_1;
static CallBack on_conformed_2;
static CallBack on_failed;
void conform_event_init(CallBack on_co1,CallBack on_co2, CallBack on_fa)
{
    on_conformed_1 = on_co1;
    on_conformed_2 = on_co2;
    on_failed = on_fa;
}

void conform_service(char *c, int sc)
{
    if(is_init) goto fail;
    static int32_t seq;
    cJSON *input = cJSON_Parse((const char *)c);

    if( !input                            ||!cJSON_HasObjectItem(input, "TYP")||
        !cJSON_HasObjectItem(input, "SEQ")||!cJSON_HasObjectItem(input, "KEY")) 
    goto fail;
    
    int32_t TYP = cJSON_GetObjectItem(input, "TYP")->valueint;
    int32_t SEQ = cJSON_GetObjectItem(input, "SEQ")->valueint;
    int32_t KEY = cJSON_GetObjectItem(input, "KEY")->valueint;

    switch (sc)
    {
    case 0:
        if (TYP == 0 && KEY==confirm_key)
        {
            on_conformed_1(input);
            // seq = SEQ;
        }
    case 1:
        if (TYP == 0 && SEQ == seq + 2 && KEY == confirm_key && cJSON_HasObjectItem(input, "PORT"))
        {
            on_conformed_2(input);
            if(!cJSON_HasObjectItem(input, "PORT")) goto fail;
            else assigned_port = cJSON_GetObjectItem(input,"PORT")->valueint;
        }
    case 2:
        break;
    }
    cJSON_Delete(input);
    return;

    fail:
    on_failed(NULL);
}