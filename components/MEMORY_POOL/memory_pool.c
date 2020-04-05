#include "memory_pool.h"

xMemoryPoolHandle xMemoryPoolCreate(int16_t block_number, int16_t block_size)
{
    xMemoryPoolHandle xMemoryPool = (xMemoryPoolHandle)malloc(sizeof(xMemoryPool_t));
    xMemoryPool->size = block_number;
    xMemoryPool->buf_size = block_size;
    xMemoryPool->xFreeList = xQueueCreate(block_number, sizeof(xMemoryBlockHandle));
    for (int i = 0; i < block_number; i++)
    {
        // 创建代码块
        xMemoryBlockHandle xMemoryBlock = (xMemoryBlockHandle)malloc(sizeof(xMemoryBlock_t));
        // 开辟空间
        xMemoryBlock->mem = (void *)malloc(block_size);
        // 初始化有效空间
        xMemoryBlock->vaild_size = 0xffff;
        // 送入待用队列
        xQueueSend(xMemoryPool->xFreeList, &xMemoryBlock, 0);
    }
    return xMemoryPool;
}

// 暂时用不到， 不写了
void xMemoryPoolDelete(xMemoryPoolHandle xMemoryPool)
{
}

xMemoryBlockHandle xMemoryBlockGet(xMemoryPoolHandle xMemoryPool, TickType_t xTicksToWait)
{
    xMemoryBlockHandle xMemoryBlock;
    if(xQueueReceive(xMemoryPool->xFreeList, &xMemoryBlock, xTicksToWait))
    {
        return xMemoryBlock;
    }
    return NULL;
}

BaseType_t xMemoryBlockRelease(xMemoryPoolHandle xMemoryPool, xMemoryBlockHandle xMemoryBlock)
{
    return xQueueSend(xMemoryPool->xFreeList, &xMemoryBlock, 0);
}
