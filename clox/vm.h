#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"

typedef struct
{
  Chunk *chunk;

  // 当虚拟机运行字节码时，它会记录它在哪里——即当前执行的指令所在的位置
  uint8_t* ip;
} VM;
typedef enum
{
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(Chunk *chunk);
#endif