#ifndef clox_scanner_h
#define clox_scanner_h
typedef enum
{
    /* 单字符标点 Single-character punctuators */
    TOKEN_LEFT_PAREN,  // 左小括号 '('
    TOKEN_RIGHT_PAREN, // 右小括号 ')'
    TOKEN_LEFT_BRACE,  // 左大括号 '{'
    TOKEN_RIGHT_BRACE, // 右大括号 '}'
    TOKEN_COMMA,       // 逗号 ','
    TOKEN_DOT,         // 点号/成员访问 '.'
    TOKEN_MINUS,       // 减号 '-'
    TOKEN_PLUS,        // 加号 '+'
    TOKEN_SEMICOLON,   // 分号 ';'
    TOKEN_SLASH,       // 斜杠 '/'
    TOKEN_STAR,        // 星号 '*'

    /* 一或两字符运算符 1~2 character operators */
    TOKEN_BANG,          // 逻辑非 '!'
    TOKEN_BANG_EQUAL,    // 不等 '!='
    TOKEN_EQUAL,         // 赋值 '='
    TOKEN_EQUAL_EQUAL,   // 相等 '=='
    TOKEN_GREATER,       // 大于 '>'
    TOKEN_GREATER_EQUAL, // 大于等于 '>='
    TOKEN_LESS,          // 小于 '<'
    TOKEN_LESS_EQUAL,    // 小于等于 '<='

    /* 字面量 Literals */
    TOKEN_IDENTIFIER, // 标识符
    TOKEN_STRING,     // 字符串字面量
    TOKEN_NUMBER,     // 数字字面量

    /* 关键字 Keywords */
    TOKEN_AND,    // 逻辑与 'and'
    TOKEN_CLASS,  // 类 'class'
    TOKEN_ELSE,   // 否则 'else'
    TOKEN_FALSE,  // 布尔假 'false'
    TOKEN_FOR,    // 循环 'for'
    TOKEN_FUN,    // 函数 'fun'
    TOKEN_IF,     // 如果 'if'
    TOKEN_NIL,    // 空值 'nil'
    TOKEN_OR,     // 逻辑或 'or'
    TOKEN_PRINT,  // 打印 'print'
    TOKEN_RETURN, // 返回 'return'
    TOKEN_SUPER,  // 父类 'super'
    TOKEN_THIS,   // 自身 'this'
    TOKEN_TRUE,   // 布尔真 'true'
    TOKEN_VAR,    // 变量声明 'var'
    TOKEN_WHILE,  // 循环 'while'

    /* 特殊状态 Special states */
    TOKEN_ERROR, // 词法错误
    TOKEN_EOF    // 文件结束
} TokenType;

typedef struct
{
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;
void initScanner(const char *source);
Token scanToken();
#endif