创造一门编程语言
在今天来看似乎不是很流行了

故事的开始还得从2年前说起
​彼时我主讲过一个分享会
主题是:初窥编译原理
从​the-super-tiny-compiler开始
介绍babel原理与使用
用​astexplorer展示AST与应用
​但是我的讲稿还设计了一章
​创造一门语言: Crafting Interpreters
当时练习时长太短，没敢讲
今天终于鼓起勇气再介绍一遍
![讲稿​配图](image.png)

Crafting Interpreters介绍
学会创造一门编程语言的基础
1. 了解一些编译原理基础概念
2. 熟悉一些静态语言和动态语言，这里推荐c++  rust  typescript
3. 本书主要基于Java和c基础实现
   
在这里你会接触到
1. scanner把字符流变 Token 流
2. parser沿着CFG去递归下降
3. Interpreter如何执行代码
4. 了解函数运行上下文与闭包
5. vscode是怎么发现return之后的dead code
6. 等等……
不剧透了，祝各位玩得开心
最后还有些资料，可以去gitee搜索查看


​
