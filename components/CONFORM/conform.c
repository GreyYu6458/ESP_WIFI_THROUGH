#include "conform.h"

enum conform_status
{
    LINK_FIRST = 0,
    LINK_SECOND,
    LINKING
};

/* 这是UDPObject的方法 TODO 将针对UDP的回调改为任意IO类型 */
static void udp_conform_callback(xUDPTaskHandle this, void* data,struct sockaddr_in* sourceAddr);

xAuthenticateTaskHandle xAuthenticateTaskCreate(xUDPTaskHandle adapter)
{
    xAuthenticateTaskHandle newObject = (xAuthenticateTaskHandle)malloc(sizeof(xAuthenticateTask_t));
    newObject->udp_adapter = adapter;
    newObject->udp_adapter->RecCallback = udp_conform_callback;
    return newObject;
}

void xAuthenticateTaskStart(xAuthenticateTaskHandle xAuthenticateTask)
{
    xUDPTaskStart(xAuthenticateTask->udp_adapter);
}


/* 服务器地址，在第一次被扫描时初始化 */
static int32_t seq_last;
static int32_t seq;
static struct sockaddr_in  serviceAddr;
static struct sockaddr_in  tmpAddr_last;
static struct sockaddr_in  tmpAddr;

/* 事件调用 */
static char on_con_1(void* result, xUDPTaskHandle this);
static char on_con_2(void* result, xUDPTaskHandle this);
static char on_linking(void* result, xUDPTaskHandle this);
static char on_fai(void* result, xUDPTaskHandle this);
static void send_device_detail(xMemoryBlockHandle d, xUDPTaskHandle this);

/* 服务器验证状态机 */
static void conform_service(xMemoryBlockHandle d, xUDPTaskHandle this)
{
    static int sc = 0;
    char *c = (char*)d->mem;
    cJSON *input = cJSON_Parse((const char *)c);
    if (!input || !cJSON_HasObjectItem(input, "TYP") ||
        !cJSON_HasObjectItem(input, "SEQ") || !cJSON_HasObjectItem(input, "KEY"))
        goto fail;
    int32_t TYP = cJSON_GetObjectItem(input, "TYP")->valueint;
    int32_t KEY = cJSON_GetObjectItem(input, "KEY")->valueint;
    seq_last = seq;
    seq = cJSON_GetObjectItem(input, "SEQ")->valueint;
    if (TYP != 0 || KEY != secret_key)
        goto fail;
    xUDPTaskSetRemote(this, &tmpAddr);
    switch (sc)
    {
    case LINK_FIRST:
        if (!on_con_1(input,this))
        {
            goto fail;
        }
        sc++;
        break;
    case LINK_SECOND:
        if (!on_con_2(input,this))
        {
            goto fail;
        }
        sc++;
        break;
    case LINKING:
        if(!on_linking(input, this))
        {
            goto fail;
        }
        break;
    }
    cJSON_ReplaceItemInObject(device_detail_json, "SEQ", cJSON_CreateNumber(seq + 1));
    // 重新利用这段内存
    send_device_detail(d, this);
    cJSON_Delete(input);
    return;

fail:
    sc = 0;
    on_fai(input, this);
    cJSON_Delete(input);
}

void send_device_detail(xMemoryBlockHandle d, xUDPTaskHandle this)
{
    get_char_detail_buff(d->mem, this->xMPool->buf_size);
    d->vaild_size = strlen((char*)d->mem);
    xUDPTaskSend(this, d);
}

inline void udp_conform_callback(xUDPTaskHandle this, void* data,struct sockaddr_in* sourceAddr)
{
    xMemoryBlockHandle d = (xMemoryBlockHandle)data;
    // 收到的JSON数据作为字符串
    ((char*)d->mem)[d->vaild_size] = 0;
    // 更新
    tmpAddr_last = tmpAddr;
    tmpAddr = *sourceAddr;
    conform_service(d, this);
}

static char on_con_1(void* result, xUDPTaskHandle this)
{
    return 1;
}

static char on_con_2(void* result, xUDPTaskHandle this)
{
    if(!cJSON_HasObjectItem((cJSON*)result, "PORT")) return 0;
    if(seq_last+2 != seq) return 0;
    // 两次服务器不相同
    if(tmpAddr.sin_addr.s_addr != tmpAddr_last.sin_addr.s_addr) return 0;
    // 获取指定的PORT
    // int16_t port = cJSON_GetObjectItem((cJSON*)result,"PORT")->valueint;
    
    /* TODO 开始桥模式
    int hsocket = socket_init(port);
    serviceAddr = sourceAddr;
    serviceAddr->sin_port = htons(assigned_port);
    AuthenticateTask* udp_task = AuthenticateTask_Consturct(hsocket, serviceAddr);
    udp_task->rec_task_start(udp_task);
    */
    return 1;
}

static char on_linking(void* result, xUDPTaskHandle this)
{
    // 两次服务器不相同
    if(tmpAddr.sin_addr.s_addr != tmpAddr_last.sin_addr.s_addr) return 0;
    return 1;
}

static char on_fai(void* result, xUDPTaskHandle this)
{
    return 1;
}





