#include <stdlib.h>
#include "compiler.h"
#include "memory.h"
#include "vm.h"
#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

void *reallocate(void *pointer, size_t oldSize, size_t newSize)
{
  // 每当我们调用reallocate()来获取更多内存时，都会强制运行一次回收
  // 这个if检查是因为，在释放或收缩分配的内存时也会调用reallocate()。
  // 我们不希望在这种时候触发GC——特别是因为GC本身也会调用reallocate()来释放内存
  if (newSize > oldSize)
  {
#ifdef DEBUG_STRESS_GC
    collectGarbage();
#endif
  }

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

void markObject(Obj *object)
{
  if (object == NULL)
    return;

#ifdef DEBUG_LOG_GC
  printf("%p mark ", (void *)object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif
  object->isMarked = true;
}

void markValue(Value value)
{
  if (IS_OBJ(value))
    markObject(AS_OBJ(value));
}
static void freeObject(Obj *object)
{
#ifdef DEBUG_LOG_GC
  printf("%p free type %d\n", (void *)object, object->type);
#endif
  switch (object->type)
  {
  case OBJ_CLOSURE:
  {
    // ObjClosure并不拥有ObjUpvalue本身，但它确实拥有包含指向这些上值的指针的数组。
    ObjClosure *closure = (ObjClosure *)object;
    FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalueCount);
    // 只释放ObjClosure本身，而不释放ObjFunction。这是因为闭包不拥有函数对象的内存管理权
    // 可能会有多个闭包都引用了同一个函数，但没有一个闭包声称对该函数有任何特殊的权限。
    // 我们不能释放某个ObjFunction，直到引用它的所有对象全部消失——甚至包括那些常量表中包含该函数的外围函数。
    // 要跟踪这个信息听起来很棘手，事实也的确如此！这就是我们很快就会写一个垃圾收集器来管理它们的原因
    FREE(ObjClosure, object);
    break;
  }
  case OBJ_FUNCTION:
  {
    ObjFunction *function = (ObjFunction *)object;
    freeChunk(&function->chunk);
    FREE(ObjFunction, object);
    break;
  }
  case OBJ_NATIVE:
    FREE(ObjNative, object);
    break;
  case OBJ_STRING:
  {
    ObjString *string = (ObjString *)object;
    FREE_ARRAY(char, string->chars, string->length + 1);
    FREE(ObjString, object);
    break;
  }
  case OBJ_UPVALUE:
    // 多个闭包可以关闭同一个变量，所以ObjUpvalue并不拥有它引用的变量。因此，唯一需要释放的就是ObjUpvalue本身。
    FREE(ObjUpvalue, object);
    break;
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
static void markRoots()
{
  for (Value *slot = vm.stack; slot < vm.stackTop; slot++)
  {
    markValue(*slot);
  }
  for (int i = 0; i < vm.frameCount; i++)
  {
    markObject((Obj *)vm.frames[i].closure);
  }

  for (ObjUpvalue *upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next)
  {
    markObject((Obj *)upvalue);
  }
  markTable(&vm.globals);
  markCompilerRoots();
}

void collectGarbage()
{
#ifdef DEBUG_LOG_GC
  printf("-- gc begin\n");
#endif
  markRoots();
#ifdef DEBUG_LOG_GC
  printf("-- gc end\n");
#endif
}