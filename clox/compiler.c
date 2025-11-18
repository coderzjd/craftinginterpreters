#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

void compile(const char *source)
{
    initScanner(source);
    int line = -1;
    for (;;)
    {
        Token token = scanToken();
        if (token.line != line)
        {
            printf("%4d ", token.line);
            line = token.line;
        }
        else
        {
            printf("   | ");
        }
        //  token.type 对应 TokenType 枚举类的int表示
        //  token.length, token.start 传参给 %.*s 用于截取字符串展示
        //  把 %.*s 拆开看就明白了：
        //      一个 * 出现在“精度”位置（点号后面）时，精度值由调用栈再取一个 int 实参；
        //      随后的 s 正常再取一个 char * 实参。
        // %s 的完整语法
        //      %[flags][width][.precision][length-modifier]s
        //      width —— 总占多少列（不够补空格）
        //      .precision —— 最多取多少字符（字符串专属含义）
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF)
            break;
    }
}