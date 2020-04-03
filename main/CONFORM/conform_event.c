#include "conform_event.h"

/* 服务器地址，在第一次被扫描时初始化 */
static int32_t seq;
static struct sockaddr_in*  serviceAddr;
static struct sockaddr_in*  tmpAddr;

char on_con_1(void* result, struct sockaddr_in* sourceAddr)
{
    int32_t SEQ = cJSON_GetObjectItem((cJSON*)result, "SEQ")->valueint;
    if(!sourceAddr) return 0;
    tmpAddr = sourceAddr;
    seq = SEQ;
    return 1;
}

char on_con_2(void* result, struct sockaddr_in* sourceAddr)
{
    int32_t SEQ = cJSON_GetObjectItem((cJSON*)result, "SEQ")->valueint;

    if(SEQ != seq || !sourceAddr) return 0;
    if(!cJSON_HasObjectItem((cJSON*)result, "PORT")) return 0;
    if(tmpAddr->sin_addr.s_addr != sourceAddr->sin_addr.s_addr) return 0;

    int hsocket = socket_init(60001 + selfID);
    serviceAddr = sourceAddr;
    UDPObject* udp_task = UDPObject_Consturct(hsocket, serviceAddr);
    udp_task->rec_task_start(udp_task);

    return 1;
}

char on_linking(void* result, struct sockaddr_in* sourceAddr)
{

}

char on_fai(void* result, struct sockaddr_in* sourceAddr)
{

}