#include <stdlib.h>

#include "memory.h"
#include "vm.h"
void *reallocate(void *pointer, size_t oldSize, size_t newSize)
{
    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }

    // realloc 是 C 标准库给的“改大小”函数——原地扩/缩，或另搬新家
    void *result = realloc(pointer, newSize);
    if (result == NULL)
    {
        exit(1);
    }
    return result;
}
static void freeObject(Obj* object) {
  switch (object->type) {
    case OBJ_CLOSURE: {
      // 只释放ObjClosure本身，而不释放ObjFunction。这是因为闭包不拥有函数对象的内存管理权
      // 可能会有多个闭包都引用了同一个函数，但没有一个闭包声称对该函数有任何特殊的权限。
      // 我们不能释放某个ObjFunction，直到引用它的所有对象全部消失——甚至包括那些常量表中包含该函数的外围函数。
      // 要跟踪这个信息听起来很棘手，事实也的确如此！这就是我们很快就会写一个垃圾收集器来管理它们的原因
      FREE(ObjClosure, object);
      break;
    }
    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      freeChunk(&function->chunk);
      FREE(ObjFunction, object);
      break;
    }
    case OBJ_NATIVE:
      FREE(ObjNative, object);
      break;
    case OBJ_STRING: {
      ObjString* string = (ObjString*)object;
      FREE_ARRAY(char, string->chars, string->length + 1);
      FREE(ObjString, object);
      break;
    }
  }
}
// 沿着链表释放所有对象
void freeObjects()
{
    Obj *object = vm.objects;
    while (object != NULL)
    {
        Obj *next = object->next;
        freeObject(object);
        object = next;
    }
}