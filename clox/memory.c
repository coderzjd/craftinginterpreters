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
    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      freeChunk(&function->chunk);
      FREE(ObjFunction, object);
      break;
    }
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