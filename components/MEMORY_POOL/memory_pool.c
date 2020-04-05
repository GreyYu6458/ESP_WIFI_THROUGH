#include "memory_pool.h"

memory_pool *memory_pool_init(int16_t block_size, int16_t block_number)
{
    memory_pool *newPool = (memory_pool *)malloc(sizeof(memory_pool));
    newPool->size = block_number;
    newPool->used_block = NULL;
    newPool->free_block = (memory_block *)malloc(sizeof(memory_block));
    newPool->free_block->memory = (indefiniteData *)malloc(sizeof(indefiniteData));
    newPool->free_block->prev = NULL;
    newPool->free_block->size = block_size;
    newPool->free_block->memory->data = (char *)malloc(block_size);

    memory_block *last_block = newPool->free_block;
    for (int i = 0; i < block_number - 1; i++)
    {
        // 构建下一个节点
        last_block->next = (memory_block *)malloc(sizeof(memory_block));
        last_block->next->memory = (indefiniteData *)malloc(sizeof(indefiniteData));
        // 开辟内存块的空间
        last_block->next->memory->data = (char *)malloc(block_size);
        // 大小
        last_block->next->size = block_size;
        // 先创建节点的前一个节点
        last_block->next->prev = last_block;

        last_block = last_block->next;
    }
    last_block->next = NULL;
    return newPool;
}

void memory_pool_free(memory_pool *mpool)
{
    memory_block *block;
    block = mpool->free_block;
    while (block != NULL)
    {
        memory_block *next = block->next;
        free(next->memory->data);
        free(next->memory);
        free(block);
        block = next;
    }
    block = mpool->used_block;
    while (block != NULL)
    {
        memory_block *next = block->next;
        free(next->memory->data);
        free(next->memory);
        free(block);
        block = next;
    }
}

memory_block *get_memory_block(memory_pool *mpool)
{
    memory_block *block = mpool->free_block;
    if (block != NULL)
    {
        // 从待用列表头删第一个
        mpool->free_block = mpool->free_block->next;
        // 头插到已用链表
        memory_block *tmp = mpool->used_block;
        mpool->used_block = block;
        block->next = tmp;
        return block;
    }
    return NULL;
}

// 不提供检测....
void release_memory_block(memory_pool *mpool, memory_block *block)
{
    memory_block *used_next = block->next;
    memory_block *used_prev = block->prev;
    // 头插到待用节点
    memory_block *free_header = mpool->free_block;
    block->next = free_header;
    mpool->free_block = block;
    // 从已用节点删除
    if (used_prev == NULL)
    {
        mpool->used_block->next = used_next;
        used_next = mpool->used_block;
    }
    else
    {
        used_prev->next = used_next;
        used_next->prev = used_prev;
    }
}

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

xMemoryBlockHandle xMemoryBlockGet(xMemoryPoolHandle xMemoryPool)
{
    xMemoryBlockHandle xMemoryBlock;
    if(xQueueReceive(xMemoryPool->xFreeList, &xMemoryBlock, 10 / portTICK_RATE_MS))
    {
        return xMemoryBlock;
    }
    return NULL;
}

BaseType_t xMemoryBlockRelease(xMemoryPoolHandle xMemoryPool, xMemoryBlockHandle xMemoryBlock)
{
    return xQueueSend(xMemoryPool->xFreeList, &xMemoryBlock, 0);
}
