#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "table.h"
#include "value.h"
// 获取OBJ类型
#define OBJ_TYPE(value) (AS_OBJ(value)->type)
// 我们用一个宏来检查某个值是否类对象OBJ_BOUND_METHOD
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
// 我们用一个宏来检查某个值是否类对象OBJ_CLASS。
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
// 我们用一个宏来检查某个值是否闭包。
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
// 确保你的值实际上是一个函数
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
// 确保你的值实际上是一个OBJ_INSTANCE
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
// 我们用一个宏来检查某个值是否本地函数。
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
// 通过c语言结构体内存对齐特性实现继承
#define IS_STRING(value) isObjType(value, OBJ_STRING)

// Value安全地转换为一个ObjBoundMethod指针
#define AS_BOUND_METHOD(value) ((ObjBoundMethod *)AS_OBJ(value))
// Value安全地转换为一个ObjClass指针
#define AS_CLASS(value) ((ObjClass *)AS_OBJ(value))
// Value安全地转换为一个ObjClosure指针
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
// Value安全地转换为一个ObjFunction指针
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
// Value安全地转换为一个ObjInstance指针
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
// Value安全地转换为一个ObjNative指针本地函数的Value中提取C函数指针
#define AS_NATIVE(value) \
  (((ObjNative *)AS_OBJ(value))->function)
// 接受一个Value返回 ObjString* 指针
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
// // 接受一个Value返回 字符数组本身
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)
typedef enum
{
  OBJ_BOUND_METHOD,
  OBJ_CLASS,
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_INSTANCE,
  OBJ_NATIVE,
  OBJ_STRING,
  OBJ_UPVALUE
} ObjType;

struct Obj
{
  ObjType type;
  // 标记垃圾回收器是否已经标记了这个对象
  bool isMarked;
  // 创建一个链表存储每个Obj。虚拟机可以遍历这个列表，找到在堆上分配的每一个对象
  struct Obj *next;
};

typedef struct
{
  Obj obj;
  // arity字段存储了函数所需要的参数数量
  int arity;
  // 函数体：字节码块
  // 记录上值个数
  int upvalueCount;
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

typedef struct ObjUpvalue
{
  Obj obj;
  // 上值捕获Value数组指针
  Value *location;
  // 当上值从栈上退出移到堆上时，closed字段保存了它的实际值
  Value closed;
  // 使用链表指向下一元素
  struct ObjUpvalue *next;
} ObjUpvalue;

// 闭包对象
typedef struct
{
  Obj obj;
  ObjFunction *function;
  // 不同的闭包可能会有不同数量的上值，所以我们需要一个动态数组。
  // 上值本身也是动态分配的，因此我们最终需要一个二级指针——一个指向动态分配的上值指针数组的指针
  ObjUpvalue **upvalues;
  // 存储数组中的元素数量
  int upvalueCount;
} ObjClosure;

typedef struct
{
  Obj obj;
  ObjString *name;
  Table methods;
} ObjClass;

typedef struct
{
  Obj obj;
  ObjClass *klass;
  Table fields;
} ObjInstance;
typedef struct
{
  Obj obj;
  // receiver：调用这个方法时，this 应该指向的具体实例。
  Value receiver;
  // method：类里定义的那个原始闭包（ObjClosure *），也就是方法本身的字节码和常量表。
  ObjClosure *method;
} ObjBoundMethod;

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method);
ObjClass *newClass(ObjString *name);
ObjClosure *newClosure(ObjFunction *function);
ObjFunction *newFunction();
ObjInstance *newInstance(ObjClass *klass);
ObjNative *newNative(NativeFn function);
ObjString *takeString(char *chars, int length);
ObjString *copyString(const char *chars, int length);
ObjUpvalue *newUpvalue(Value *slot);
void printObject(Value value);
// static inline 就不会触发多重定义，还能让编译器自由内联省掉 .o 文件和链接这一步
// 高频、超短、零状态” 的小函数，用 static inline 扔到头文件里，是 C 世界里最常见、最合理的写法
static inline bool isObjType(Value value, ObjType type)
{
  // isObjType判断是OBJ类型且类型匹配
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}
#endif