
// 阶段 0：
// 纯递归——连加 1+2+…+10
const foo1 = () => {
    function sumRec(n) {
        if (n === 1) return 1;
        return n + sumRec(n - 1);
    }
    console.log('rec ', sumRec(10)); // 55
}
foo1()
// 阶段 1：把递归改成while循环（两个变量就够）
// 核心：原来每层栈帧保存的“当前 n”被 i 替代，返回值用 res 累加。
const foo2 = () => {
    function sumLoop(n) {
        let res = 0;
        let i = 1;
        while (i <= n) {
            res += i;
            ++i
        }
        return res;
    }
    console.log('whileloop', sumLoop(10)); // 55
}
foo2()
// 阶段 2：递归里加入减法（左结合）
// 文法：expr ::= num ('+' num | '-' num)*

const foo3 = () => {
    //  Token 流：['1','+','2','-','3','+','4']
    const tok = ['1', '+', '2', '-', '3', '+', '4'];
    let idx = 0
    let cur = () => tok[idx]
    let adv = () => idx++;

    function exprRec() {
        let left = +cur()
        adv()
        while (idx < tok.length) {
            const op = cur();
            adv();
            const right = +cur();
            adv();
            left = op === '+' ? left + right : left - right;
        }
        return left;
    }
    console.log('rec±', exprRec()); // 1+2-3+4 = 4
}
foo3()
// 阶段 3：把“加减循环”抽象成带门槛的通用 parse
const foo4 = () => {
    // 先只支持 + -，门槛统一 1，结合方向用“门槛+1”体现左结合。
    const tok2 = ['1', '+', '2', '-', '3', '+', '4'];
    let idx2 = 0
    let cur2 = () => tok2[idx2]
    let adv2 = () => idx2++;
    const prec = { '+': 1, '-': 1 };

    function parse(minPrec) {
        let left = +cur2(); adv2();
        while (idx2 < tok2.length) {
            const op = cur2();
            const opPrec = prec[op];
            if (opPrec < minPrec) break;   // 优先级不够，收手
            adv2();
            const right = parse(opPrec + 1); // 左结合：门槛+1
            left = op === '+' ? left + right : left - right;
        }
        return left;
    }
    console.log('loop±', parse(0)); // 4
}
foo4()
// 阶段 4：引入 * 与 优先级
const foo = () => {
    const tok2 = ['1', '+', '2', '*', '3', '+', '4'];
    let idx2 = 0
    let cur2 = () => tok2[idx2]
    let adv2 = () => idx2++;
    const prec = { '+': 1, '*': 2 };

    function parse(minPrec) {
        let left = +cur2(); adv2();
        while (idx2 < tok2.length) {
            const op = cur2();
            const opPrec = prec[op];
            if (opPrec < minPrec) break;   // 优先级不够，收手
            adv2();
            const right = parse(opPrec + 1); // 左结合：门槛+1
            left = op === '+' ? left + right : left * right;
        }
        return left;
    }
    console.log('loop*+', parse(0)); // 4
}
foo()
// 阶段5 压栈
const bar = () => {
    const tok2 = ['1', '+', '2', '*', '3', '+', '4'];
    let idx = 0;
    const cur = () => tok2[idx];
    const adv = () => idx++;
    const prec = { '+': 1, '*': 2 };

    const code = [];          // 输出字节码
    const opStack = [];       // 只存运算符和它的门槛

    function parse(minPrec) {
        code.push({ op: 'PUSH', val: +cur() });
        adv();
        while (idx < tok2.length) {
            const op = cur();
            const opPrec = prec[op];
            if (opPrec < minPrec) break;
            adv();

            // 左结合：门槛+1；右结合：门槛不变
            const nextMin = opPrec + 1;   // 左结合
            opStack.push({ op, prec: opPrec });
            parse(nextMin);               // 啃右部

            // 右部回来，栈顶=右值，次顶=左值
            // 发指令，弹二压一（结果当新左值）
            code.push({ op });
        }
    }

    parse(0);
    console.log(code);
    /* 输出：
    [
      { op: 'PUSH', val: 1 },
      { op: 'PUSH', val: 2 },
      { op: 'PUSH', val: 3 },
      { op: 'MUL' },
      { op: 'ADD' },
      { op: 'PUSH', val: 4 },
      { op: 'ADD' }
    ]
    */
    //vm执行顺序
    // 1. PUSH 1     [1]
    // 2. PUSH 2     [1, 2]
    // 3. PUSH 3     [1, 2, 3]
    // 4. MUL        [1, 6]      ← 2*3=6
    // 5. ADD        [7]         ← 1+6=7
    // 6. PUSH 4     [7, 4]
    // 7. ADD        [11]        ← 7+4=11
};
bar();