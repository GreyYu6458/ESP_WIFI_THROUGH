
#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_
#include "msg_def.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "queue.h"

/************************************ 使用freeRTOS实现内存池 ************************************/
typedef struct
{
    int16_t vaild_size; // 有效数据
    void* mem;
}xMemoryBlock_t;
typedef xMemoryBlock_t* xMemoryBlockHandle;

typedef struct
{
    int16_t size;     // 有多少内存块
    int16_t buf_size; // 开辟的空间大小
    xQueueHandle xFreeList;
}xMemoryPool_t;
typedef xMemoryPool_t* xMemoryPoolHandle;


extern xMemoryPoolHandle xMemoryPoolCreate(int16_t block_number, int16_t block_size);

extern void xMemoryPoolDelete(xMemoryPoolHandle xMemoryPool);

extern xMemoryBlockHandle xMemoryBlockGet(xMemoryPoolHandle xMemoryPool, TickType_t xTicksToWait);

extern BaseType_t xMemoryBlockRelease(xMemoryPoolHandle xMemoryPool, xMemoryBlockHandle xMemoryBlock);

#endif