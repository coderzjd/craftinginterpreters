package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;

class LoxInstance {
    private LoxClass klass;
    private final Map<String, Object> fields = new HashMap<>();

    LoxInstance(LoxClass klass) {
        this.klass = klass;
    }

    Object get(Token name) {
        if (fields.containsKey(name.lexeme)) {
            return fields.get(name.lexeme);
        }
        LoxFunction method = klass.findMethod(name.lexeme);
        if (method != null) {
            return method.bind(this);
        }
        // 如果这个实例中不包含给定名称的属性和方法，会发生什么。我们可以悄悄返回一些假值，如nil，
        // 但是根据我对JavaScript等语言的经验，这种行为只是掩盖了错误，而没有做任何有用的事。
        // 相反，我们将它作为一个运行时错误。??
        throw new RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
    }

    void set(Token name, Object value) {
        fields.put(name.lexeme, value);
    }

    @Override
    public String toString() {
        return klass.name + " instance";
    }
}