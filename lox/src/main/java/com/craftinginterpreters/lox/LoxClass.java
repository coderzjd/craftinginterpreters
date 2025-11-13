package com.craftinginterpreters.lox;

import java.util.List;
import java.util.Map;

class LoxClass implements LoxCallable {
    final String name;
    private final Map<String, LoxFunction> methods;

    LoxClass(String name, Map<String, LoxFunction> methods) {
        this.name = name;
        this.methods = methods;
    }

    LoxFunction findMethod(String name) {
        if (methods.containsKey(name)) {
            return methods.get(name);
        }
        return null;
    }

    @Override
    public String toString() {
        return name;
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        LoxInstance instance = new LoxInstance(this);
        return instance;
    }

    @Override
    public int arity() {
        return 0;
    }
}
// LoxClass 与 LoxInstance 梳理
// 1、LoxClass 类本身存储了name和 methods 
    // class Bacon {
    //   eat() {
    //     print "Crunch crunch crunch!";
    //   }
    // }
    // 获得一个class : {name: Bacon, methods:[eat] }
// 2、LoxClass实现了LoxCallable 可以直接 call方法实例化LoxInstance。
    // Bacon() 
    // 获得一个Bacon实例：{klass: Bacon,fields:[存储setter的字段]}