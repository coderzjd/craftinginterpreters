#ifndef clox_vm_h
#define clox_vm_h

#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

// 一个CallFrame代表一个正在进行的函数调用
typedef struct
{
  // 一个指向被调用闭包的指针
  ObjClosure *closure;
  // 返回地址存储在被调用者的帧中 ip 等到从函数中返回时，虚拟机会跳转到调用方的CallFrame的ip，并从那里继续执行
  uint8_t *ip;
  // slots字段指向虚拟机的值栈中该函数可以使用的第一个槽
  Value *slots;
} CallFrame;
// 新增部分结束

typedef struct
{
  // frames字段是一个CallFrame数组，表示函数调用栈
  CallFrame frames[FRAMES_MAX];
  // frameCount字段存储了CallFrame栈的当前高度——正在进行的函数调用的数量
  int frameCount;
  // vm的stack存放运行时的值
  Value stack[STACK_MAX];
  // stackTop指向下一个值要被压入的位置
  Value *stackTop;
  // 全局变量存储
  Table globals;
  // 字符串驻留
  Table strings;
  // class 初始化init方法字符串常量
  ObjString *initString;
  // openUpvalues 存储指向打开的上值链表的头
  ObjUpvalue *openUpvalues;
  // bytesAllocated 是虚拟机已分配的托管内存实时字节总数
  size_t bytesAllocated;
  // nextGC 是触发下一次回收的阈值
  size_t nextGC;
  // 存储一个指向表头的指针，链表中的每个对象都有一个指向下一个对象的指针
  Obj *objects;

  // grayCount 字段存储grayStack数组中的当前元素数量
  int grayCount;
  // grayCapacity 字段存储grayStack数组的当前容量
  int grayCapacity;
  // grayStack 跟踪所有的灰色对象
  Obj **grayStack;
} VM;
typedef enum
{
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;
// 导出vm变量，供其他模块使用
extern VM vm;
void initVM();
void freeVM();
InterpretResult interpret(const char *source);
void push(Value value);
Value pop();
#endif