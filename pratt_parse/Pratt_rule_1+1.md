# 编译 `1+1` 字节码逐步记录

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

---

### 步骤 4 决定处理中缀，advance 吃掉 '+'`  
无新指令  
Chunk：`[OP_CONSTANT, 0]`

---

### 步骤 5 binary() 递归右操作数 parsePrecedence(7)  
递归里 advance 吃第二个 NUMBER → number()  
`emitConstant(1.0)`  
Chunk：`[OP_CONSTANT, 0, OP_CONSTANT, 1]`

---

### 步骤 6 递归层无更高优先级，返回  
无 emit  
Chunk保持：`[OP_CONSTANT, 0, OP_CONSTANT, 1]`

---

### 步骤 7 回到 binary() 尾部  
`emitByte(OP_ADD)`  
Chunk：`[OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD]`

---

### 步骤 8 endCompiler()  
`emitReturn()`  
最终字节码：
```bash
OP_CONSTANT 0
OP_CONSTANT 1
OP_ADD
OP_RETURN
```
### 步骤 9 VM 执行顺序

PC 顺序执行，操作数栈（初始空）变化如下：

| PC | 指令 | 栈变化 | 执行后栈 |
|----|------|--------|----------|
| 0 | `OP_CONSTANT 0` | push 常量 #0 (1.0) | [1] |
| 2 | `OP_CONSTANT 1` | push 常量 #1 (1.0) | [1, 1] |
| 4 | `OP_ADD` | pop→1, pop→1, push 2 | [2] |
| 5 | `OP_RETURN` | pop→结果, 返回调用者 | [] |

结果：2