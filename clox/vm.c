#include <stdio.h>
#include "common.h"
#include "debug.h"
#include "vm.h"

VM vm;

void initVM()
{
}

void freeVM()
{
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

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
            disassembleInstruction(vm.chunk,(int)(vm.ip - vm.chunk->code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_CONSTANT:
        {
            Value constant = READ_CONSTANT();
            printValue(constant);
            printf("\n");
            break;
        }
        case OP_RETURN:
        {
            return INTERPRET_OK;
        }
        }
    }
    // 在函数退出之前 #undef READ_BYTE，外部就无法使用这个宏了
#undef READ_BYTE
#undef READ_CONSTANT
}
InterpretResult interpret(Chunk *chunk)
{
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}