#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum
{
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
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
    // 偏移量查找对应的行号。
    int *lines;
    // constants 存放的是编译时候产生的常量值
    ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);
#endif