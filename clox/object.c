#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"
#define ALLOCATE_OBJ(type, objectType) \
    (type *)allocateObject(sizeof(type), objectType)

static Obj *allocateObject(size_t size, ObjType type)
{
    Obj *object = (Obj *)reallocate(NULL, 0, size);
    object->type = type;
    // 手动维护单链表： 每当我们分配一个Obj时，就将其插入到列表中
    object->next = vm.objects;
    vm.objects = object;
    return object;
}
static ObjString *allocateString(char *chars, int length, uint32_t hash)
{
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    tableSet(&vm.strings, string, NIL_VAL);
    return string;
}
// 这是工业级“快而散”的 FNV-1a 32 位哈希；两个常量是规范魔法数，循环里“异或+乘”就能把任意字节序列均匀地搅成 32 位哈希值。
// 异或——把新字节的“局部变化”砸进当前哈希的低位；
// 乘法——用一个大素数（或演示里的小乘数）把低位变化扩散到整个 32 位寄存器，完成雪崩。
// 循环下去，每个字符都经历“先异或再扩散”，最后得到分布均匀的 32 位哈希值。

// 2166136261u
//      FNV 作者先选定一个大素数做“偏移基数”（offset basis），要求是大于 2³² 的下一个素数附近、二进制里 0/1 分布均衡的数，让初始状态“既大又乱”。
//      随后用几十万个英文单词、路径名、源码标识符做碰撞测试，最终把“碰撞最少”的那个固定下来——所以它是数学筛选＋实测调优的结果。
// 16777619
//      理论：选素数是为了乘法逆元存在，且低位全为 1（0x01000193），保证进位链最长，能把低位变化迅速卷到高位。
//      实践：同样拿海量真实数据跑桶分布，发现它比邻近素数冲突少、雪崩快，才写进规范。
// → 先定数学标准（素数、逆元、位分布）→ 再用统计学验证碰撞率
static uint32_t hashString(const char *key, int length)
{
    // 无符号32位
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++)
    {
        // (uint8_t)key[i] 就是把 char 的 1 个字节当成无符号 8 位整数取出来，范围 0~255。
        // C 的整型提升规则（integer promotion）会自动把 uint8_t 先零扩展成 32 位，再跟 uint32_t 做异或，
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}
ObjString *takeString(char *chars, int length)
{
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL)
    {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocateString(chars, length, hash);
}
ObjString *copyString(const char *chars, int length)
{
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL)
    {
        return interned;
    }
    char *heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}
void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    }
}