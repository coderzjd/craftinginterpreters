package com.craftinginterpreters.lox;

enum TokenType {
    // 单字符记号
    LEFT_PAREN,     // 左括号
    RIGHT_PAREN,    // 右括号
    LEFT_BRACE,     // 左花括号
    RIGHT_BRACE,    // 右花括号
    COMMA,          // 逗号
    DOT,            // 点
    MINUS,          // 减号
    PLUS,           // 加号
    SEMICOLON,      // 分号
    SLASH,          // 斜杠
    STAR,           // 星号

    // 一两个字符的记号
    BANG,           // 感叹号
    BANG_EQUAL,     // 不等于
    EQUAL,          // 赋值号
    EQUAL_EQUAL,    // 等于
    GREATER,        // 大于
    GREATER_EQUAL,  // 大于等于
    LESS,           // 小于
    LESS_EQUAL,     // 小于等于

    // 字面量
    IDENTIFIER,     // 标识符
    STRING,         // 字符串
    NUMBER,         // 数字

    // 关键字
    AND,            // 与
    CLASS,          // 类
    ELSE,           // 否则
    FALSE,          // 假
    FUN,            // 函数
    FOR,            // 循环
    IF,             // 如果
    NIL,            // 空值
    OR,             // 或
    PRINT,          // 打印
    RETURN,         // 返回
    SUPER,          // 父类
    THIS,           // 当前对象
    TRUE,           // 真
    VAR,            // 变量
    WHILE,          // 当循环

    EOF             // 文件结束
}