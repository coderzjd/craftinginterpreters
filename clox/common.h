#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
// 我们将为垃圾回收器添加一个可选的“压力测试”模式。当定义这个标志后，GC就会尽可能频繁地运行
#define DEBUG_STRESS_GC
// 启用这个功能后，当clox使用动态内存执行某些操作时，会将信息打印到控制台
#define DEBUG_LOG_GC
#define UINT8_COUNT (UINT8_MAX + 1)
#endif