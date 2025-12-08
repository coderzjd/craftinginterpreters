#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"
// 获取OBJ类型
#define OBJ_TYPE(value) (AS_OBJ(value)->type)
// 我们用一个宏来检查某个值是否闭包。
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
// 确保你的值实际上是一个函数
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
// 我们用一个宏来检查某个值是否本地函数。
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
// 通过c语言结构体内存对齐特性实现继承
#define IS_STRING(value) isObjType(value, OBJ_STRING)
// Value安全地转换为一个ObjClosure指针
#define AS_CLOSURE(value)      ((ObjClosure*)AS_OBJ(value))
// Value安全地转换为一个ObjFunction指针
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
// Value安全地转换为一个ObjNative指针本地函数的Value中提取C函数指针
#define AS_NATIVE(value) \
  (((ObjNative *)AS_OBJ(value))->function)
// 接受一个Value返回 ObjString* 指针
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
// // 接受一个Value返回 字符数组本身
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)
typedef enum
{
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_STRING,
} ObjType;

struct Obj
{
  ObjType type;
  // 创建一个链表存储每个Obj。虚拟机可以遍历这个列表，找到在堆上分配的每一个对象
  struct Obj *next;
};

typedef struct
{
  Obj obj;
  // arity字段存储了函数所需要的参数数量
  int arity;
  // 函数体：字节码块
  Chunk chunk;
  // 存储函数名称
  ObjString *name;
} ObjFunction;

// 添加本地函数
typedef Value (*NativeFn)(int argCount, Value *args);
typedef struct
{
  Obj obj;
  NativeFn function;
} ObjNative;

struct ObjString
{
  Obj obj;
  int length;
  char *chars;
  uint32_t hash;
};
// 闭包对象
typedef struct
{
  Obj obj;
  ObjFunction *function;
} ObjClosure;

ObjClosure *newClosure(ObjFunction *function);
ObjFunction *newFunction();
ObjNative *newNative(NativeFn function);
ObjString *takeString(char *chars, int length);
ObjString *copyString(const char *chars, int length);
void printObject(Value value);
// static inline 就不会触发多重定义，还能让编译器自由内联省掉 .o 文件和链接这一步
// 高频、超短、零状态” 的小函数，用 static inline 扔到头文件里，是 C 世界里最常见、最合理的写法
static inline bool isObjType(Value value, ObjType type)
{
  // isObjType判断是OBJ类型且类型匹配
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}
#endif