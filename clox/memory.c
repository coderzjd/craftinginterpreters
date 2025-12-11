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
  // 如果对象已经被标记，我们就不会再标记它，因此也不会把它添加到灰色栈中。这就保证了已经是灰色的对象不会被重复添加，
  // 而且黑色对象不会无意中变回灰色。换句话说，它使得波前只通过白色对象向前移动
  if (object->isMarked)
    return;

#ifdef DEBUG_LOG_GC
  printf("%p mark ", (void *)object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif
  object->isMarked = true;

  if (vm.grayCapacity < vm.grayCount + 1)
  {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack = (Obj **)realloc(vm.grayStack, sizeof(Obj *) * vm.grayCapacity);

    if (vm.grayStack == NULL)
    {
      // 我们对这个数组负担全部责任，其中包括分配失败。如果我们不能创建或扩张灰色栈，那我们就无法完成垃圾回收
      exit(1);
    }
  }
  vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value)
{
  if (IS_OBJ(value))
    markObject(AS_OBJ(value));
}

static void markArray(ValueArray *array)
{
  for (int i = 0; i < array->count; i++)
  {
    markValue(array->values[i]);
  }
}
static void blackenObject(Obj *object)
{
#ifdef DEBUG_LOG_GC
  printf("%p blacken ", (void *)object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif
  switch (object->type)
  {
  case OBJ_CLOSURE:
  {
    ObjClosure *closure = (ObjClosure *)object;
    markObject((Obj *)closure->function);
    for (int i = 0; i < closure->upvalueCount; i++)
    {
      markObject((Obj *)closure->upvalues[i]);
    }
    break;
  }
  case OBJ_FUNCTION:
  {
    ObjFunction *function = (ObjFunction *)object;
    markObject((Obj *)function->name);
    markArray(&function->chunk.constants);
    break;
  }
  case OBJ_UPVALUE:
    markValue(((ObjUpvalue *)object)->closed);
    break;
  case OBJ_NATIVE:
  case OBJ_STRING:
    break;
  }
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
  // 当VM关闭时，我们需要释放它。
  free(vm.grayStack);
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

static void traceReferences()
{
  while (vm.grayCount > 0)
  {
    Obj *object = vm.grayStack[--vm.grayCount];
    blackenObject(object);
  }
}

static void sweep()
{
  Obj *previous = NULL;
  Obj *object = vm.objects;
  // 外层的while循环会遍历堆中每个对象组成的链表，检查它们的标记位。
  // 如果某个对象被标记（黑色），我们就不管它，继续进行。
  // 如果它没有被标记（白色），我们将它从链表中断开，并使用我们已经写好的freeObject()函数释放它
  while (object != NULL)
  {
    if (object->isMarked)
    {
      object->isMarked = false;
      previous = object;
      object = object->next;
    }
    else
    {
      Obj *unreached = object;
      object = object->next;
      if (previous != NULL)
      {
        previous->next = object;
      }
      else
      {
        vm.objects = object;
      }

      freeObject(unreached);
    }
  }
}

void collectGarbage()
{
#ifdef DEBUG_LOG_GC
  printf("-- gc begin\n");
#endif
  // 标记根
  markRoots();
  // 标记阶段
  traceReferences();
  // 标记表中的字符串: 需要特殊处理
  tableRemoveWhite(&vm.strings);
  // 回收
  sweep();
#ifdef DEBUG_LOG_GC
  printf("-- gc end\n");
#endif
}