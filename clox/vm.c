#include <stdarg.h>
#include <stdio.h>
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

VM vm;
static void resetStack()
{
    vm.stackTop = vm.stack;
}
static void runtimeError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}
void initVM()
{
    resetStack();
}

void freeVM()
{
}
void push(Value value)
{
    *vm.stackTop = value;
    //  指向下一个空位置
    vm.stackTop++;
}
Value pop()
{
    vm.stackTop--;
    return *vm.stackTop;
}
static Value peek(int distance)
{
    // 在 C 里，指针就是数组的通用接口，[] 只是 *(ptr + offset) 的“甜语法”，负数、正数都能用，只要别越界。
    // int a[] = {1, 2, 3};
    // int *q = a;
    // *(q + 1) 等价于 q[1]
    return vm.stackTop[-1 - distance];
}
static bool isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}
// | 写在                  | 作用域             | 链接属性                |
// | ---------------      | ----------         | ------------------- |
// | 函数定义前加 `static` | 当前 `.c` 文件     | **内部链接**（本文件私有）     |
// | 全局变量前加 `static` | 当前 `.c` 文件     | **内部链接**（本文件私有）     |
// | 局部变量加 `static`   | 所在代码块         | **静态存储期**（函数返回也不销毁） |
static InterpretResult run()
{
    // 宏定义：读取当前指令并将指令指针前移一位:  READ_BYTE() 相当于 *vm.ip++
#define READ_BYTE() (*vm.ip++)
    // 宏定义：读取一个常量值
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op)                        \
    do                                                  \
    {                                                   \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
        {                                               \
            runtimeError("Operands must be numbers.");  \
            return INTERPRET_RUNTIME_ERROR;             \
        }                                               \
        double b = AS_NUMBER(pop());                    \
        double a = AS_NUMBER(pop());                    \
        push(valueType(a op b));                        \
    } while (false)
    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        printf("  vm'stack is ");
        for (Value *slot = vm.stack; slot < vm.stackTop; slot++)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_CONSTANT:
        {
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        }
        case OP_NIL:
            push(NIL_VAL);
            break;
        case OP_TRUE:
            push(BOOL_VAL(true));
            break;
        case OP_FALSE:
            push(BOOL_VAL(false));
            break;
        case OP_EQUAL:
        {
            Value b = pop();
            Value a = pop();
            push(BOOL_VAL(valuesEqual(a, b)));
            break;
        }
        case OP_GREATER:
            BINARY_OP(BOOL_VAL, >);
            break;
        case OP_LESS:
            BINARY_OP(BOOL_VAL, <);
            break;
        case OP_ADD:
            BINARY_OP(NUMBER_VAL, +);
            break;
        case OP_SUBTRACT:
            BINARY_OP(NUMBER_VAL, -);
            break;
        case OP_MULTIPLY:
            BINARY_OP(NUMBER_VAL, *);
            break;
        case OP_DIVIDE:
            BINARY_OP(NUMBER_VAL, /);
            break;
        case OP_NOT:
            push(BOOL_VAL(isFalsey(pop())));
            break;
        case OP_NEGATE:
            if (!IS_NUMBER(peek(0)))
            {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(NUMBER_VAL(-AS_NUMBER(pop())));
            break;
        case OP_RETURN:
        {
            printValue(pop());
            printf("\n");
            return INTERPRET_OK;
        }
        }
    }
    // 在函数退出之前 #undef READ_BYTE，外部就无法使用这个宏了
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}
InterpretResult interpret(const char *source)
{
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk))
    {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    printf("vm is runing !\n");
    InterpretResult result = run();
    freeChunk(&chunk);
    return result;
}