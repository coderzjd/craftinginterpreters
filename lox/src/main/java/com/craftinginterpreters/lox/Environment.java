package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;

class Environment {
    // 在每个环境中添加一个对其外围环境的引用
    final Environment enclosing;
    private final Map<String, Object> values = new HashMap<>();

    // 无参构造函数用于全局作用域环境，它是环境链的结束点
    Environment() {
        enclosing = null;
    }

    // 用来创建一个嵌套在给定外部作用域内的新的局部作用域
    Environment(Environment enclosing) {
        this.enclosing = enclosing;
    }

    Object get(Token name) {
        if (values.containsKey(name.lexeme)) {
            return values.get(name.lexeme);
        }
        if (enclosing != null) {
            return enclosing.get(name);
        }
        throw new RuntimeError(name,
                "Undefined variable '" + name.lexeme + "'.");
    }

    void assign(Token name, Object value) {
        if (values.containsKey(name.lexeme)) {
            values.put(name.lexeme, value);
            return;
        }
        if (enclosing != null) {
            enclosing.assign(name, value);
            return;
        }
        throw new RuntimeError(name,
                "Undefined variable '" + name.lexeme + "'.");
    }

    // 允许重定义——至少对于全局变量如此
    void define(String name, Object value) {
        values.put(name, value);
    }
}