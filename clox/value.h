#ifndef clox_value_h
#define clox_value_h

#include "common.h"
// 头文件要“自给自足”且“最小公开”；h文件引入可能导致“污染范围”（因为h文件可能被外部引用）
// .h 里 #include → “我这个头文件本身就需要它”（类型、宏、内联函数用到别人）
// c 里 #include → “仅我这个实现文件需要它”（只在 .c 里用，头文件里用不到）

// value.c里面#include "object.h"，但是value.h没有#include "object.h"，
// 导致value.h里用到Obj类型时编译器报错找不到定义
// 添加前向声明
typedef struct Obj Obj;
typedef struct ObjString ObjString;
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
        Obj* obj;
    } as;
} Value;
// 定义了几个宏来检查 Value 的类型
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_OBJ(value)     ((value).type == VAL_OBJ)

// Value 解包并恢复出 C 值
#define AS_OBJ(value)     ((value).as.obj)
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

// 将原生 C 值提升为 Value
#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL           ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object)   ((Value){VAL_OBJ, {.obj = (Obj*)object}})

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