package com.craftinginterpreters.lox;

import java.util.List;

class LoxFunction implements LoxCallable {
    private final Stmt.Function declaration;
    private final Environment closure;

    LoxFunction(Stmt.Function declaration, Environment closure) {
        this.closure = closure;
        this.declaration = declaration;
    }

    LoxFunction bind(LoxInstance instance) {
        // 给函数绑定this就是创建一个新的环境，再把这个环境作为函数的新闭包
        // 这个环境的父环境是函数原来的闭包
        // 然后在这个新环境中定义this，指向传入的实例
        // 最后返回一个新的LoxFunction，使用这个新环境作为闭包
        // 这样，当函数体内访问this时，会在新环境中找到它
        Environment environment = new Environment(closure);
        environment.define("this", instance);
        return new LoxFunction(declaration, environment);
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        // 每个函数都会维护自己的环境，其中存储着那些变量
        // 函数递归
        // 通过在执行函数主体时使用不同的环境，用同样的代码调用相同的函数可以产生不同的结果
        Environment environment = new Environment(closure);
        for (int i = 0; i < declaration.params.size(); i++) {
            environment.define(declaration.params.get(i).lexeme, arguments.get(i));
        }
        // 拿到通过异常抛出的值
        try {
            interpreter.executeBlock(declaration.body, environment);
        } catch (Return returnValue) {
            return returnValue.value;
        }
        return null;
    }

    @Override
    public int arity() {
        return declaration.params.size();
    }

    @Override
    public String toString() {
        return "<fn " + declaration.name.lexeme + ">";
    }
}