#include "conform.h"

/* 服务器地址，在第一次被扫描时初始化 */
static int32_t seq_last;
static int32_t seq;
static struct sockaddr_in*  serviceAddr;
static struct sockaddr_in*  tmpAddr_last;
static struct sockaddr_in*  tmpAddr;

/* 事件调用 */
static char on_con_1(void* result, AuthenticateTask *this);
static char on_con_2(void* result, AuthenticateTask *this);
static char on_linking(void* result, AuthenticateTask *this);
static char on_fai(void* result, AuthenticateTask *this);

/* 这是UDPObject的方法 TODO 将针对UDP的回调改为任意IO类型 */
static void udp_conform_callback(void* data,struct sockaddr_in* sourceAddr, UDPObject* this);

/* 验证任务开始 */
static void authenticate_task_start(AuthenticateTask * this);



/* 该类构造函数 */
AuthenticateTask* AuthenticateTask_Consturct(UDPObject* adapter)
{
    AuthenticateTask* newObject = (AuthenticateTask*)malloc(sizeof(AuthenticateTask));
    newObject->this = newObject;
    newObject->udp_adapter = adapter;
    newObject->udp_adapter->rec_callback = udp_conform_callback;
}

/* 服务器验证状态机 */
static void conform_service(char *c, AuthenticateTask *this)
{
    static int sc = 0;
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
    send_device_detail(this);
    cJSON_Delete(input);
    return;

fail:
    sc = 0;
    on_failed(input, this);
    cJSON_Delete(input);
}

void send_device_detail(AuthenticateTask* this)
{
    char* jsonData = get_char_detail();
    indefiniteData dataWrap = {.len = strlen(jsonData), .data = jsonData};
    this->udp_adapter->send(&dataWrap,this->udp_adapter);
}

inline void udp_conform_callback(void *data,struct sockaddr_in* sourceAddr, UDPObject* this)
{
    indefiniteData *d = (indefiniteData *)data;
    // 收到的JSON数据作为字符串
    d->data[d->len] = 0;
    // 更新
    tmpAddr_last = tmpAddr;
    tmpAddr = sourceAddr;
    conform_service(d->data, this);
}

static char on_con_1(void* result, AuthenticateTask *this)
{
    return 1;
}

static char on_con_2(void* result, AuthenticateTask *this)
{
    if(!cJSON_HasObjectItem((cJSON*)result, "PORT")) return 0;
    if(seq_last != seq + 2 || !tmpAddr) return 0;
    // 两次服务器不相同
    if(tmpAddr->sin_addr.s_addr != tmpAddr_last->sin_addr.s_addr) return 0;
    // 获取指定的PORT
    int16_t port = cJSON_GetObjectItem((cJSON*)result,"PORT")->valueint;
    
    /* TODO 开始桥模式
    int hsocket = socket_init(port);
    serviceAddr = sourceAddr;
    serviceAddr->sin_port = htons(assigned_port);
    AuthenticateTask* udp_task = AuthenticateTask_Consturct(hsocket, serviceAddr);
    udp_task->rec_task_start(udp_task);
    */
    return 1;
}

static char on_linking(void* result, AuthenticateTask *this)
{
    // 两次服务器不相同
    if(tmpAddr->sin_addr.s_addr != tmpAddr_last->sin_addr.s_addr) return 0;
    return 1;
}

static char on_fai(void* result, AuthenticateTask *this)
{

}

static void authenticate_task_start(AuthenticateTask * this)
{
    this->udp_adapter->rec_task_start(this->udp_adapter);
}