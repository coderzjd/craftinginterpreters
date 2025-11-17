#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
// 虚拟机一个固定的栈大小
// 可能会压入太多的值并耗尽栈空间——典型的“堆栈溢出”错误
#define STACK_MAX 256
typedef struct
{
  Chunk *chunk;

  // 当虚拟机运行字节码时，它会记录它在哪里——即当前执行的指令所在的位置
  uint8_t *ip;
  Value stack[STACK_MAX];
  // stackTop指向下一个值要被压入的位置
  Value *stackTop;
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
void push(Value value);
Value pop();
#endif