package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;
// 在解析器生成语法树之后，解释器执行语法树之前，我们会对语法树再进行一次遍历
// 这次遍历的目的是确定每个变量引用到底指向哪个作用域中的变量声明
// 这个过程叫做静态分析(static analysis)，因为它发生在程序运行之前

class Resolver implements Expr.Visitor<Void>, Stmt.Visitor<Void> {
    private final Interpreter interpreter;
    private final Stack<Map<String, Boolean>> scopes = new Stack<>();

    Resolver(Interpreter interpreter) {
        this.interpreter = interpreter;
    }

    void resolve(List<Stmt> statements) {
        for (Stmt statement : statements) {
            resolve(statement);
        }
    }

    @Override
    public Void visitBlockStmt(Stmt.Block stmt) {
        beginScope();
        resolve(stmt.statements);
        endScope();
        return null;
    }

    @Override
    public Void visitVarStmt(Stmt.Var stmt) {
        declare(stmt.name);
        if (stmt.initializer != null) {
            resolve(stmt.initializer);
        }
        define(stmt.name);
        return null;
    }

    private void resolve(Stmt stmt) {
        stmt.accept(this);
    }

    private void resolve(Expr expr) {
        expr.accept(this);
    }

    private void beginScope() {
        scopes.push(new HashMap<String, Boolean>());
    }

    private void endScope() {
        scopes.pop();
    }
}

// 变量解析工作就像一个小型的解释器。它会遍历整棵树，访问每个节点，但是静态分析与动态执行还是不同的
// 1、没有副作用。当静态分析处理一个print语句时，它并不会打印任何东西。对本地函数或其它与外部世界联系的操作也会被终止，并且没有任何effect
// 2、没有控制流。循环只会被处理一次，if语句中的两个分支都会处理，逻辑操作符也不会做短路处理

// 有几个节点是比较特殊的
// 1、块语句为它所包含的语句引入了一个新的作用域
// 2、函数声明为其函数体引入了一个新的作用域，并在该作用域中绑定了它的形参
// 3、变量声明将一个新变量加入到当前作用域中。
// 4、变量定义和赋值表达式需要解析它们的变量值
// 5、其余的节点不做任何特别的事情，但是我们仍然需要为它们实现visit方法，以遍历其子树。
// 尽管+表达式本身没有任何变量需要解析，但是它的任一操作数都可能需要