#ifndef clox_value_h
#define clox_value_h
#include <string.h>
#include "common.h"
// 头文件要“自给自足”且“最小公开”；h文件引入可能导致“污染范围”（因为h文件可能被外部引用）
// .h 里 #include → “我这个头文件本身就需要它”（类型、宏、内联函数用到别人）
// c 里 #include → “仅我这个实现文件需要它”（只在 .c 里用，头文件里用不到）

// value.c里面#include "object.h"，但是value.h没有#include "object.h"，
// 导致value.h里用到Obj类型时编译器报错找不到定义
// 添加前向声明
typedef struct Obj Obj;
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

// NaN-boxing 总结（一句话版 + 三步版）
// 一句话
// 把「小数据+类型标签」全部塞进 IEEE-754 NaN 的 51 位废位，64 bit 一值到底，靠位运算完成拆装，省内存、省分支、省缓存。
// 三步实现
// 借位：选固定 NaN 位模式 0x7FF8...，低 51 bit 当仓库。
// 装箱：
// value = (payload & 48_bit_mask) | (tag << 48) | NaN_mask
// 一条或指令完成。
// 拆箱：
// tag = value >> 48
// payload = value & 48_bit_mask
// 一条移位+一条与指令完成。
// 三大优化
// 密度：值数组 64 bit/元素，缓存行装 8 个，命中率↑
// 速度：纯寄存器位运算，无分支，流水线不断
// 内存：小整数/布尔/指针免堆分配，GC 压力↓

#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define QNAN ((uint64_t)0x7ffc000000000000)
#define TAG_NIL 1   // 01.
#define TAG_FALSE 2 // 10.
#define TAG_TRUE 3  // 11.

typedef uint64_t Value;
#define IS_BOOL(value) (((value) | 1) == TRUE_VAL)
#define IS_NIL(value) ((value) == NIL_VAL)
#define IS_NUMBER(value) (((value) & QNAN) != QNAN)
#define IS_OBJ(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_NUMBER(value) valueToNum(value)
#define AS_OBJ(value) ((Obj *)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(b) ((b) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL ((Value)(uint64_t)(QNAN | TAG_NIL))
#define NUMBER_VAL(num) numToValue(num)
#define OBJ_VAL(obj) (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

static inline double valueToNum(Value value)
{
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}
static inline Value numToValue(double num)
{
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}
#else

typedef enum
{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

typedef struct
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        Obj *obj;
    } as;
} Value;
// 定义了几个宏来检查 Value 的类型
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

// Value 解包并恢复出 C 值
#define AS_OBJ(value) ((value).as.obj)
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

// 将原生 C 值提升为 Value
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj *)object}})

#endif
typedef struct
{
    int capacity;
    int count;
    Value *values;
} ValueArray;
bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);
#endif