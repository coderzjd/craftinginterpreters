#include <stdarg.h>
#include <stdio.h>
#include "common.h"
#include <string.h>
#include <time.h>
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

VM vm;
static Value clockNative(int argCount, Value *args)
{
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}
static void resetStack()
{
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
}
static void runtimeError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // 打印报错调用栈
    for (int i = vm.frameCount - 1; i >= 0; i--)
    {
        CallFrame *frame = &vm.frames[i];
        ObjFunction *function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
        if (function->name == NULL)
        {
            fprintf(stderr, "script\n");
        }
        else
        {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
    resetStack();
}

static void defineNative(const char *name, NativeFn function)
{
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}
void initVM()
{
    resetStack();
    vm.objects = NULL;
    vm.grayCount = 0;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);
    // 添加本地函数
    defineNative("clock", clockNative);
}

void freeVM()
{
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeObjects();
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

// 切换函数执行CallFrames上下文
static bool call(ObjClosure *closure, int argCount)
{
    // 函数参数个数拦截校验
    if (argCount != closure->function->arity)
    {
        runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }
    // CallFrame数组具有固定的大小，我们需要确保一个深的调用链不会溢
    if (vm.frameCount == FRAMES_MAX)
    {
        runtimeError("Stack overflow.");
        return false;
    }
    CallFrame *frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount)
{
    if (IS_OBJ(callee))
    {
        switch (OBJ_TYPE(callee))
        {
        case OBJ_CLOSURE:
            return call(AS_CLOSURE(callee), argCount);
        case OBJ_NATIVE:
        {
            // 如果被调用的对象是一个本地函数，我们就会立即调用C函数。
            // 没有必要使用CallFrames或其它任何东西。
            // 我们只需要交给C语言，得到结果，然后把结果塞回栈中。这使得本地函数的运行速度能够尽可能快
            NativeFn native = AS_NATIVE(callee);
            Value result = native(argCount, vm.stackTop - argCount);
            vm.stackTop -= argCount + 1;
            push(result);
            return true;
        }
        default:
            break; // Non-callable object type.
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}

static ObjUpvalue *captureUpvalue(Value *local)
{
    ObjUpvalue *prevUpvalue = NULL;
    ObjUpvalue *upvalue = vm.openUpvalues;
    // 我们从列表的头部开始，
    // 我们有三个原因可以退出循环：
    // 1.我们停止时的局部变量槽是我们要找的槽。我在找到了一个现有的上值捕获了这个变量，因此我们重用这个上值
    // 2.我们找不到需要搜索的上值了。当upvalue为NULL时，这意味着列表中每个开放上值都指向位于我们要找的槽之上的局部变量，或者（更可能是）上值列表是空的。无论怎样，我们都没有找到对应该槽的上值
    // 3.我们找到了一个上值，其局部变量槽低于我们正查找的槽位。因为列表是有序的，这意味着我们已经超过了正在关闭的槽，因此肯定没有对应该槽的已有上值。
    while (upvalue != NULL && upvalue->location > local)
    {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local)
    {
        return upvalue;
    }

    ObjUpvalue *createdUpvalue = newUpvalue(local);
    // 只需要添加代码将上值插入到列表中。我们退出列表遍历的原因，要么是到达了列表末尾，要么是停在了第一个栈槽低于待查找槽位的上值
    createdUpvalue->next = upvalue;
    if (prevUpvalue == NULL)
    {
        vm.openUpvalues = createdUpvalue;
    }
    else
    {
        prevUpvalue->next = createdUpvalue;
    }
    // VM现在可以确保每个指定的局部变量槽都只有一个ObjUpvalue。如果两个闭包捕获了相同的变量，它们会得到相同的上值
    return createdUpvalue;
}

// closeUpvalues 只干一件事儿：把“还指向栈、且地址 ≥ last 这一级”的所有 open upvalue 节点，一次性搬离栈、永久落户到堆。
static void closeUpvalues(Value *last)
{
    // 1、last 是即将消失的那一段栈的“起始地址
    // 2、链表按 location 降序排列，头插法 → 越靠头的节点，location 值越大。
    // 3、从链表头开始，只要节点还指向 ≥ last 的栈地址，就说明它马上会变成悬空指针，必须处理。
    // 4、因为降序，一旦遇到 < last 的节点，后面全安全，可立即结束循环。
    // 5、取出当前头节点（location 最大）。
    // 6、把栈上的值拷贝到堆里的 closed 字段——“搬家”第一步。
    // 7、把 location 指针改指向自己的 closed 字段——从此脱离栈，后续读写都走堆。
    // 8、把头节点摘掉，继续处理下一个可能也 ≥ last 的节点。
    while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last)
    {
        ObjUpvalue *upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}
// 场景1：遇到 } 结束任意局部作用域（if / while / for / block）。
// 只关当前栈顶那一个 slot（stackTop - 1），保证刚死亡的局部变量立即从 open 链表移除，不干扰后续代码。

// 场景2：整个函数要返回，整帧即将 pop。
// 把当前函数的全部 slots（从 frame->slots 到 stackTop）一次性关掉，防止帧销毁后还有 upvalue 悬空指向废栈。

// fun f() {
//   var a = 1;        // slot 0
//   {                 // 新作用域
//     var b = 2;      // slot 1
//     fun g() { print a; print b; }
//   }                 // ← 这里 emit OP_CLOSE_UPVALUE，只关 slot 1（b）
//   return a;         // ← 函数返回前，slot 0（a） 仍指向栈，但帧马上要 pop
// }
// OP_CLOSE_UPVALUE 只把 b 搬堆；
// a 还留在 slot 0，没人帮它提前关；
// 函数返回时整帧销毁，如果不把 a 也搬堆，闭包 g 就悬空。
// 因此 OP_RETURN 必须兜底批量关——从 frame->slots 到 stackTop 之间所有仍 open 的 upvalue 一次全搬走，保证帧 pop 后没有遗留指针指向废栈
static bool isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}
static void concatenate()
{
    ObjString *b = AS_STRING(peek(0));
    ObjString *a = AS_STRING(peek(1));
    // 赋值原始两个字符串之后， a 和 b 目前仍然活在堆里，这段代码并没有释放它们，等待GC回收
    int length = a->length + b->length;
    char *chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString *result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}
// | 写在                  | 作用域             | 链接属性                |
// | ---------------      | ----------         | ------------------- |
// | 函数定义前加 `static` | 当前 `.c` 文件     | **内部链接**（本文件私有）     |
// | 全局变量前加 `static` | 当前 `.c` 文件     | **内部链接**（本文件私有）     |
// | 局部变量加 `static`   | 所在代码块         | **静态存储期**（函数返回也不销毁） |
static InterpretResult run()
{
    CallFrame *frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)

#define READ_SHORT() \
    (frame->ip += 2, \
     (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())
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
        disassembleInstruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code));
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
        case OP_POP:
            pop();
            break;
        case OP_GET_LOCAL:
        {
            uint8_t slot = READ_BYTE();
            push(frame->slots[slot]);
            break;
        }
        case OP_SET_LOCAL:
        {
            uint8_t slot = READ_BYTE();
            frame->slots[slot] = peek(0);
            break;
        }
        case OP_GET_GLOBAL:
        {
            ObjString *name = READ_STRING();
            Value value;
            if (!tableGet(&vm.globals, name, &value))
            {
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            push(value);
            break;
        }
        case OP_DEFINE_GLOBAL:
        {
            ObjString *name = READ_STRING();
            tableSet(&vm.globals, name, peek(0));
            pop();
            break;
        }
        case OP_SET_GLOBAL:
        {
            ObjString *name = READ_STRING();
            if (tableSet(&vm.globals, name, peek(0)))
            {
                tableDelete(&vm.globals, name);
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_GET_UPVALUE:
        {
            uint8_t slot = READ_BYTE();
            push(*frame->closure->upvalues[slot]->location);
            break;
        }
        case OP_SET_UPVALUE:
        {
            uint8_t slot = READ_BYTE();
            *frame->closure->upvalues[slot]->location = peek(0);
            break;
        }
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
        {
            if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
            {
                concatenate();
            }
            else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
            {
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a + b));
            }
            else
            {
                runtimeError(
                    "Operands must be two numbers or two strings.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
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
        case OP_PRINT:
        {
            printValue(pop());
            printf("\n");
            break;
        }
        case OP_JUMP:
        {
            uint16_t offset = READ_SHORT();
            frame->ip += offset;
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            // 还原字节码存储的偏移量
            uint16_t offset = READ_SHORT();
            if (isFalsey(peek(0)))
                frame->ip += offset;
            break;
        }
        case OP_LOOP:
        {
            uint16_t offset = READ_SHORT();
            frame->ip -= offset;
            break;
        }
        case OP_CALL:
        {
            int argCount = READ_BYTE();
            if (!callValue(peek(argCount), argCount))
            {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        case OP_CLOSURE:
        {
            ObjFunction *function = AS_FUNCTION(READ_CONSTANT());
            ObjClosure *closure = newClosure(function);
            push(OBJ_VAL(closure));
            // 这段代码是闭包诞生的神奇时刻。我们遍历了闭包所期望的每个上值。对于每个上值，我们读取一对操作数字节
            for (int i = 0; i < closure->upvalueCount; i++)
            {
                uint8_t isLocal = READ_BYTE();
                uint8_t index = READ_BYTE();
                if (isLocal)
                {
                    closure->upvalues[i] = captureUpvalue(frame->slots + index);
                }
                else
                {
                    closure->upvalues[i] = frame->closure->upvalues[index];
                }
            }
            break;
        }
        // 到达该指令时，我们要提取的变量就在栈顶。我们调用一个辅助函数，传入栈槽的地址。该函数负责关闭上值，并将局部变量从栈中移动到堆上
        case OP_CLOSE_UPVALUE:
            closeUpvalues(vm.stackTop - 1);
            pop();
            break;
        case OP_RETURN:
        {
            // 当函数返回一个值时，该值会在栈顶
            Value result = pop();
            // 当函数返回时，我们调用相同的辅助函数，并传入函数拥有的第一个栈槽
            closeUpvalues(frame->slots);
            vm.frameCount--;
            // 如果是最后一个CallFrame，这意味着我们已经完成了顶层代码的执行
            if (vm.frameCount == 0)
            {
                pop();
                return INTERPRET_OK;
            }
            // 否则我们把返回值压回堆栈，切换回调用者的上下文
            vm.stackTop = frame->slots;
            push(result);
            frame = &vm.frames[vm.frameCount - 1];
            break;
        }
        }
    }
    // 在函数退出之前 #undef READ_BYTE，外部就无法使用这个宏了
#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}
InterpretResult interpret(const char *source)
{
    ObjFunction *function = compile(source);
    if (function == NULL)
        return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure *closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0);
    printf("vm is runing !\n");
    return run();
}