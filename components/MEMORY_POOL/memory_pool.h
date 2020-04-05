
#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_
#include "msg_def.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "queue.h"
// #include "semphr.h"

typedef struct memory_block
{
    int16_t size;
    indefiniteData* memory;
    struct memory_block* next;
    struct memory_block* prev;
}memory_block;

typedef struct memory_pool
{
    int16_t size;
    memory_block* free_block;
    memory_block* used_block;
}memory_pool;

/* 初始化内存块 */
extern memory_pool* memory_pool_init(int16_t block_size, int16_t block_number);

/* 释放掉内存池 */
extern void memory_pool_free(memory_pool* mpool);

/* 获取内存块 */
extern memory_block* get_memory_block(memory_pool* mpool);

/* ‘释放’掉内存块 */
extern void release_memory_block(memory_pool *mpool, memory_block* block);

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

extern xMemoryBlockHandle xMemoryBlockGet(xMemoryPoolHandle xMemoryPool);

extern BaseType_t xMemoryBlockRelease(xMemoryPoolHandle xMemoryPool, xMemoryBlockHandle xMemoryBlock);

#endif