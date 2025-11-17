#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
typedef enum
{
    OP_RETURN,
} OpCode;

typedef struct
{
    // 实际使用的已分配元数数量（计数，count）
    int count;
    // 数组中已分配的元素数量（容量，capacity）
    int capacity;
    // 保存字节码的数组
    uint8_t *code;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk *chunk, uint8_t byte);
#endif