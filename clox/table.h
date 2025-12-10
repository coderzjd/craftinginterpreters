#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"
typedef struct
{
    ObjString *key;
    Value value;
} Entry;

// 哈希表是一个条目数组
typedef struct
{
    // 键/值对数量（计数，count）。
    int count;
    //   数组的分配大小（容量，capacity）
    int capacity;
    Entry *entries;
} Table;
void initTable(Table* table);
void freeTable(Table* table);
// 传入一个表和一个键。如果它找到一个带有该键的条目，则返回true，否则返回false
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableSet(Table* table, ObjString* key, Value value);
bool tableDelete(Table* table, ObjString* key);
void tableAddAll(Table* from, Table* to);
ObjString* tableFindString(Table* table, const char* chars,int length, uint32_t hash);
void markTable(Table* table);
#endif