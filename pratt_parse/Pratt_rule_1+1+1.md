# 编译 `1+1+1` 字节码逐步记录

---

### 步骤 0 初始  
Chunk：空  
`[]`

---

### 步骤 1 首次 advance（compile 里）  
仍无 emit  
Chunk：`[]`

---

### 步骤 2 parsePrecedence(1) 再 advance  
准备前缀，无 emit  
Chunk：`[]`

---

### 步骤 3 前缀回调 number()  
`emitConstant(1.0)`  
Chunk：`[OP_CONSTANT, 0]`  
（常量表 #0 = 1.0）

---

### 步骤 4 决定处理中缀，advance 吃掉第一个 '+'`  
无新指令  
Chunk：`[OP_CONSTANT, 0]`

---

### 步骤 5 binary() 递归右操作数 parsePrecedence(7)  
递归里 advance 吃第二个 NUMBER → number()  
`emitConstant(1.0)`  
Chunk：`[OP_CONSTANT, 0, OP_CONSTANT, 1]`  
（常量表 #1 = 1.0）

---

### 步骤 6 递归层无更高优先级，返回  
无 emit  
Chunk保持：`[OP_CONSTANT, 0, OP_CONSTANT, 1]`

---

### 步骤 7 回到 binary() 尾部  
`emitByte(OP_ADD)`  
Chunk：`[OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD]`

---

### 步骤 8 Pratt 主循环继续  
`precedence(1) &lt;= PLUS(6)` 仍成立，再吃第二个 '+'  
advance 消费 '+' → 无 emit  
Chunk：`[OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD]`

---

### 步骤 9 第二次 binary()  
递归 parsePrecedence(7)  
advance 吃第三个 NUMBER → number()  
`emitConstant(1.0)`  
Chunk：`[OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD, OP_CONSTANT, 2]`  
（常量表 #2 = 1.0）

---

### 步骤 10 递归层无更高优先级，返回  
无 emit  
Chunk保持：`[OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD, OP_CONSTANT, 2]`

---

### 步骤 11 回到第二次 binary() 尾部  
`emitByte(OP_ADD)`  
Chunk：`[OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD, OP_CONSTANT, 2, OP_ADD]`

---

### 步骤 12 endCompiler()  
`emitReturn()`  
最终字节码：
```bash
OP_CONSTANT 0
OP_CONSTANT 1
OP_ADD
OP_CONSTANT 2
OP_ADD
OP_RETURN
```
### 步骤 13 VM 执行顺序

| PC | 指令              | 栈变化                  | 执行后栈    |
| -- | --------------- | -------------------- | ------- |
| 0  | `OP_CONSTANT 0` | push 1.0             | \[1]    |
| 2  | `OP_CONSTANT 1` | push 1.0             | \[1, 1] |
| 4  | `OP_ADD`        | pop→1, pop→1, push 2 | \[2]    |
| 5  | `OP_CONSTANT 2` | push 1.0             | \[2, 1] |
| 7  | `OP_ADD`        | pop→1, pop→2, push 3 | \[3]    |
| 8  | `OP_RETURN`     | pop→结果, 返回调用者        | \[]     |
clox/Pratt_rule_1+1.mdclox/Pratt_rule_1+1.md
