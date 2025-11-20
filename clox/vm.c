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
#define BINARY_OP(op)     \
    do                    \
    {                     \
        double b = pop(); \
        double a = pop(); \
        push(a op b);     \
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
        case OP_ADD:
            BINARY_OP(+);
            break;
        case OP_SUBTRACT:
            BINARY_OP(-);
            break;
        case OP_MULTIPLY:
            BINARY_OP(*);
            break;
        case OP_DIVIDE:
            BINARY_OP(/);
            break;
        case OP_NEGATE:
            push(-pop());
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