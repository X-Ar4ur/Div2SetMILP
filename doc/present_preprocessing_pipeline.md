# PRESENT.cl 在进入 Div2SetMILP 之前的预处理全过程

本文以 [PRESENT.cl](/d:/Work/UCASNJ/EasyBC-2080-related-key/benchmarks/singleKey/BlockCipher/PRESENT.cl) 为例，说明 EasyBC 在进入 `Div2SetMILP` 之前，`.cl` 文件依次经历了哪些内部表示的转变，以及每一步的产物是什么。

目标不是泛泛解释“解析”和“编译”，而是回答下面这几个非常具体的问题：

- `PRESENT.cl` 是怎样被读入的？
- 哪些源码语句会变成哪些 AST 节点？
- `interpreter.generateCode(*programRoot)` 到底构造了什么？
- `allBox`、`procs`、`procedureHs` 分别是在什么时候产生的？
- 为什么进入 `Div2SetMILP` 时已经可以按 TAC 风格遍历密码过程？

## 1. 总览：从 `.cl` 到 `Div2SetMILP` 的主链路

在除法性质分析路径下，主流程位于 [main.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/main.cpp:306) 的 `DivTrailsMGR`：

1. 根据命令行里的密码名找到对应的 `.cl` 文件。
2. 调用 `yyparse()`，把 `.cl` 解析成 AST，根节点是 `programRoot`。
3. 调用 `interpreter.generateCode(*programRoot)`，遍历 AST，构造语义对象：
   - 全局环境 `env`
   - `allBox`
   - `ProcValue` / `Procedure`
4. 基于 `allBox` 先生成 S-box 的 division trails 和不等式。
5. 从 `interpreter` 里取出 `procs`，交给 `Transformer` 转换成 `procedureHs`。
6. 把 `procedureHs` 交给 `Div2SetMILP`，开始 TAC 遍历建模。

所以，对 `PRESENT.cl` 来说，进入 `Div2SetMILP` 之前，至少经历了这四层表示：

1. 源码文本 `.cl`
2. AST：`programRoot`
3. 解释器语义对象：`Value` / `BoxValue` / `ProcValue` / `Procedure`
4. TAC 风格过程表示：`ProcedureH` + `ThreeAddressNode`

## 2. 输入文件：PRESENT.cl 里有哪些结构

`PRESENT.cl` 的顶层内容很典型：

- `@cipher present_64`
- `sbox uint4[16] s = {...};`
- `pbox uint[64] p = {...};`
- 两个 `r_fn`
  - `round_function1`
  - `round_function2`
- 一个 `fn`
  - `enc`

这几类语句后面会分别变成：

- `NCipherNameDeclaration`
- `NSboxInitialization`
- `NPboxInitialization`
- `NRoundFunctionDeclaration`
- `NCipherFunctionDeclaration`

## 3. 第一步：从文本到 AST

### 3.1 入口

在 [main.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/main.cpp:318) 中：

- `yyin = fopen(filePath.c_str(), "r");`
- `yyparse();`

`yyparse()` 使用 [parser.y](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/language/parser.y) 中的语法规则，把整个 `.cl` 文件构造成一棵 AST。

根规则是：

```yacc
program
    : stmts {programRoot = $1;}
```

也就是说，最后得到的是一个 `NBlock` 根节点，所有顶层语句都挂在这个 `NBlock` 下面。

### 3.2 PRESENT.cl 顶层语句对应的 AST 节点

对本例来说，顶层 AST 大致可以理解为：

1. `NCipherNameDeclaration("present_64")`
2. `NSboxInitialization(...)`
3. `NPboxInitialization(...)`
4. `NRoundFunctionDeclaration("round_function1", ...)`
5. `NRoundFunctionDeclaration("round_function2", ...)`
6. `NCipherFunctionDeclaration("enc", ...)`

### 3.3 一个细节：`@cipher` 在解析阶段就写入了全局 `cipherName`

`@cipher present_64` 对应 `NCipherNameDeclaration`。它的构造函数在 [ASTNode.h](/d:/Work/UCASNJ/EasyBC-2080-related-key/include/preprocessing/ASTNode.h:946) 里直接执行了：

```cpp
cipherName = name;
```

所以：

- `cipherName` 的设置实际上发生在 AST 节点构造阶段
- 后面 `NCipherNameDeclaration::compute()` 只是返回 `nullptr`

这意味着 `@cipher` 更像是一个“解析时副作用声明”，而不是后续语义阶段才处理的对象。

## 4. 第二步：从 AST 到解释器语义对象

### 4.1 总入口：`generateCode(*programRoot)`

在 [main.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/main.cpp:335) 中，解析成功后执行：

```cpp
interpreter.generateCode(*programRoot);
```

`generateCode` 的实现位于 [Interpreter.h](/d:/Work/UCASNJ/EasyBC-2080-related-key/include/preprocessing/Interpreter.h:37)：

```cpp
pushBlock();
CodeGenBlockPtr topDefs = blockStack.back();
ValuePtr valuePtr = root.compute(*this);
popBlock();
```

它做的事很直接：

1. 新建顶层作用域
2. 调用 AST 根节点的 `compute()`
3. 让整个程序从上到下被“解释执行”一遍

这里的“执行”不是运行加密算法求具体密文，而是把源码转成 EasyBC 自己的语义对象。

### 4.2 `NBlock::compute()` 如何驱动整个预处理

`programRoot` 是 `NBlock`。它的 `compute()` 位于 [ASTNode.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/ASTNode.cpp:1076)：

```cpp
for(const auto& stmt : *(this->getStmtList())) {
    last = stmt->compute(interpreter);
    if(last && last->getValueType() == ValueType::VTProcValue)
        interpreter.addProc(last);
}
```

这段逻辑说明：

- 顶层语句按源码顺序逐条处理
- 如果某条语句计算结果是 `ProcValue`
  - 就把它加入 `interpreter.procs`

因此，对 `PRESENT.cl` 顶层内容来说：

- `sbox` / `pbox` 语句会更新全局环境和 `allBox`
- `r_fn` / `fn` 语句会生成 `ProcValue`
- 这些 `ProcValue` 最终会积累到 `interpreter.getProcs()`

## 5. PRESENT 中的全局 box 如何进入 `allBox`

### 5.1 `sbox uint4[16] s = {...};`

对应 `NSboxInitialization::compute()`，实现位于 [ASTNode.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/ASTNode.cpp:966)。

它做了三件事：

1. 先执行内部的变量声明 `declaration->compute(interpreter)`
2. 把整数列表转成 `vector<ValuePtr>`
3. 同时把具体整数提取成 `vector<int> sboxEles`

随后构造：

- 一个放进解释器环境 `env` 的 `BoxValue`
- 一个放进全局 `allBox` 的整数表

关键语句：

```cpp
ValuePtr sbox = std::make_shared<BoxValue>(arrayName, "sbox", rowSize, result);
interpreter.addToEnv(arrayName, sbox);
allBox["sbox" + this->declaration->getId()->getName()] = sboxEles;
```

对 PRESENT 而言：

- 环境里的名字是 `s`
- `allBox` 里的 key 是 `sboxs`
- value 是长度 16 的 `std::vector<int>`

### 5.2 `pbox uint[64] p = {...};`

对应 `NPboxInitialization::compute()`，实现位于 [ASTNode.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/ASTNode.cpp:991)。

逻辑与 S-box 类似：

- 在环境里存 `BoxValue("p", "pbox", ...)`
- 在 `allBox` 里存 `allBox["pboxp"] = pboxEles`

因此，在 `generateCode(*programRoot)` 执行结束后，`PRESENT.cl` 至少会贡献出：

```cpp
allBox["sboxs"] = {12, 5, 6, 11, ...}
allBox["pboxp"] = {0, 16, 32, 48, ...}
```

这也是后面 Step 1 和 Step 2 能遍历 `allBox` 做 S-box division trail 与不等式生成的原因。

## 6. PRESENT 中的函数如何变成 `ProcValue`

### 6.1 `r_fn round_function1` / `round_function2`

`r_fn` 对应 `NRoundFunctionDeclaration::compute()`，实现位于 [ASTNode.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/ASTNode.cpp:1116)。

它会：

1. 复制当前全局环境到一个新的过程作用域中
2. 处理三个参数
   - round number
   - round key
   - state / plaintext
3. 计算函数体 `block->compute(interpreter)`
4. 读取 `return` 的结果
5. 构造 `Procedure`
6. 再包装成 `ProcValue`
7. 打上 `isRndf` 标记

所以 `round_function1` 和 `round_function2` 在此阶段的产物不是 TAC，而是：

- `Procedure`
- 外层 `ProcValue`

### 6.2 `fn enc`

`fn` 对应 `NCipherFunctionDeclaration::compute()`，实现位于 [ASTNode.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/ASTNode.cpp:1197)。

处理方式与 `r_fn` 类似，但最后会调用：

```cpp
proc->setIsFn();
```

这很重要，因为后面 `Transformer::transformProcedures()` 会把 `isFn` 的过程重命名为 `main`，供 `Div2SetMILP` 从“主流程”开始遍历。

也就是说：

- 源码里叫 `enc`
- 进入 `procedureHs` 后会变成 `main`

## 7. 函数体内部发生了什么：以 PRESENT 的典型语句为例

这一部分最关键，因为它决定了 `Procedure` 里到底积累了什么样的“过程语义”。

### 7.1 变量声明：建立符号对象

例如：

```cl
uint1[64] n_input = input ^ key;
uint1[64] s_out;
```

`NVariableDeclaration::compute()` 会根据类型建立对应的 `Value` 或 `ArrayValue`，并放入解释器环境 `env`。

如果是数组，例如 `uint1[64] s_out;`，会为它建立：

- 名字为 `s_out` 的 `ArrayValue`
- 里面 64 个元素，每个元素是一个 `ValuePtr`

这一步得到的是“符号数组”，不是实际比特值。

### 7.2 二元运算：形成 `InternalBinValue`

例如：

```cl
uint1[64] n_input = input ^ key;
```

右侧 `input ^ key` 由 `NBinaryOperator::compute()` 处理，生成一个 `InternalBinValue`。

它不是立刻求值，而是构造一个带操作符的语义节点：

- 左操作数是 `input`
- 右操作数是 `key`
- 操作符是 `XOR`

然后赋值语句会把这个结果放入 `n_input` 对应的位置关系中。

### 7.3 `View(...)`：切片成新的 `ArrayValue`

例如：

```cl
uint1[4] sbox_in = View(n_input, i*4, i*4+3);
```

`NViewArray::compute()` 的实现位于 [ASTNode.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/ASTNode.cpp:490)。

它会：

1. 从环境中取出数组 `n_input`
2. 计算下界和上界
3. 把对应区间的元素拷出来
4. 返回一个新的 `ArrayValue`

对于 PRESENT 的 `round_function1` 来说，这一步得到的是：

- 第 `i` 个 nibble 对应的 4 比特切片
- 名义上是 `sbox_in`

### 7.4 `s<sbox_in>` / `p<s_out>`：转成 BOXOP

例如：

```cl
uint1[4] sbox_out = s<sbox_in>;
uint1[64] rtn = p<s_out>;
```

这类语句对应 `NBoxOperation::compute()`，实现位于 [ASTNode.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/ASTNode.cpp:564)。

它会构造一个：

- 左边是 `BoxValue`
  - `s` 或 `p`
- 右边是被作用的数组
  - `sbox_in` 或 `s_out`
- 操作符是 `BOXOP`

然后把这个 `InternalBinValue` 直接加入当前过程的 `sequence`：

```cpp
interpreter.addToSequence(res);
```

这一步很关键，因为后面 Transformer 正是从 `sequence` 中把它转成 TAC 节点。

### 7.5 `for` 循环：在解释阶段直接展开

例如：

```cl
for (i from 0 to 15) { ... }
for (i from 1 to 31) { ... }
```

`NForStatement::compute()` 的实现位于 [ASTNode.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/ASTNode.cpp:1238)。

这里不是保留一个“循环节点给后端再处理”，而是直接：

1. 计算 `from` 和 `to`
2. 把循环变量 `i` 写入环境
3. 在 `while` 中反复执行 `block->compute(interpreter)`

这意味着对 PRESENT：

- `round_function1` 里的 16 轮 nibble 处理在预处理阶段就被展开成 16 份效果
- `enc` 里的 `for (i from 1 to 31)` 也会在预处理阶段展开成 31 次 `round_function1(...)` 调用效果

这是理解后续 TAC 建模的关键点：

- `Div2SetMILP` 看到的不是原始“循环结构”
- 而是循环展开后的过程序列

### 7.6 函数调用：变成 `ProcCallValue` / `ProcCallValueIndex`

例如 `enc` 中：

```cl
r_plaintext = round_function1(i, View(key, (i - 1) * 64, i * 64 - 1), r_plaintext);
```

`NFunctionCall::compute()` 的实现位于 [ASTNode.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/ASTNode.cpp:258)。

它会：

1. 先计算所有实参
2. 在解释器里找到被调过程 `Procedure`
3. 构造一个 `ProcCallValue`
4. 再根据返回值类型，把它包装成：
   - 单值时：`ProcCallValueIndex`
   - 数组时：一个由多个 `ProcCallValueIndex` 组成的 `ArrayValue`

而且整个返回结果会被加入 `sequence`：

```cpp
interpreter.addToSequence(res);
```

对 PRESENT 的 `enc` 来说，这意味着：

- 每次 `round_function1(...)` 调用，不会在此处直接内联成具体语句
- 而是先记录成“过程调用返回值”的语义对象
- 后面由 Transformer 再转成 `PUSH` + `CALL` 风格的 TAC 节点

### 7.7 `return`

`NReturnStatement::compute()` 只做一件事：

```cpp
interpreter.setCurrentReturnValue(returns);
```

因此每个过程最终都会得到一个显式的返回对象：

- `round_function1` 返回 `rtn`
- `round_function2` 返回 `input`
- `enc` 返回最后的 `r_plaintext`

## 8. `generateCode()` 执行结束后，PRESENT 已经变成了什么

到这里，源码已经不再只是 AST，而是变成了解释器内部的一组语义对象。

对 PRESENT 来说，至少有下面三类关键产物。

### 8.1 `allBox`

类型定义在 [main.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/main.cpp:25)：

```cpp
std::map<std::string, std::vector<int>> allBox = {};
```

对 PRESENT 的典型内容是：

- `allBox["sboxs"]`
- `allBox["pboxp"]`

它存的是整数表，不是 AST，也不是 TAC。

### 8.2 `interpreter` 内部环境

里面保存了很多名字到语义对象的映射，例如：

- `s` -> `BoxValue`
- `p` -> `BoxValue`
- 各函数内部的局部数组和临时值

这部分更像“符号环境”。

### 8.3 `procs`

通过 `interpreter.getProcs()` 取出，类型是：

```cpp
std::vector<ProcValuePtr>
```

对 PRESENT 来说，通常会包含 3 个主要过程：

- `round_function1`
- `round_function2`
- `enc`

它们内部已经记录了：

- 参数
- 过程环境
- `sequence`
- 返回值

但它们还不是最终给 `Div2SetMILP` 直接遍历的格式。

## 9. 第三步：从 `ProcValue` 到 `ProcedureH`

在 [main.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/main.cpp:374) 里：

```cpp
std::vector<ProcValuePtr> procs = interpreter.getProcs();
Transformer transformer(procs);
transformer.transformProcedures();
std::vector<ProcedureHPtr> procedureHs = transformer.getProcedureHs();
```

这一步是进入 `Div2SetMILP` 之前最后一个关键转换。

### 9.1 Transformer 的输入是什么

输入是 `ProcValuePtr`，也就是“过程语义对象”。

它们来自前面的解释器阶段，里面有 `Procedure`，而 `Procedure` 持有：

- 参数
- block / sequence
- returns

### 9.2 Transformer 做了什么

`Transformer::transform()` 位于 [Transformer.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/Transformer.cpp:6)。

它会把 `Procedure` 里的各种 `Value` 对象变成 `ThreeAddressNode`：

- 参数 -> `NodeType::PARAMETER`
- `InternalBinValue` -> 二元三地址节点
- `InternalUnValue` -> 一元三地址节点
- `ArrayValue` -> 展开成多个节点
- `ProcCallValueIndex` -> `PUSH` / `CALL`
- `BoxValue` 相关表达式 -> `BOXOP`

因此，这一步的本质是：

- 从“语义对象图”
- 转成“更线性、更适合遍历建模的三地址表示”

### 9.3 `enc` 为什么会变成 `main`

`transformProcedures()` 位于 [Transformer.cpp](/d:/Work/UCASNJ/EasyBC-2080-related-key/lib/preprocessing/Transformer.cpp:597)。

其中有一段：

```cpp
if (ele->getProcedurePtr()->getIsFn()) {
    ele->setName("main");
}
```

所以：

- `enc` 因为被 `NCipherFunctionDeclaration` 标成了 `isFn`
- 在 Transformer 阶段会被改名为 `main`

后面 `Div2SetMILP::programGenModel()` 正是通过查找 `proc->getName() == "main"` 来找到主加密流程。

### 9.4 `ProcedureH` 长什么样

`ProcedureH` 定义在 [ProcedureH.h](/d:/Work/UCASNJ/EasyBC-2080-related-key/include/preprocessing/ProcedureH.h:17)。

它的核心字段是：

- `name`
- `parameters`
- `block`
- `returns`

其中：

- `parameters` 是 `vector<vector<ThreeAddressNodePtr>>`
- `block` 是 `vector<ThreeAddressNodePtr>`
- `returns` 是 `vector<ThreeAddressNodePtr>`

也就是说，到这一步已经非常接近典型的 TAC / 三地址中间表示了。

## 10. 针对 PRESENT，Transformer 之后通常会得到什么

以本例来看，`procedureHs` 至少可以理解为三份 TAC 风格过程：

1. `main`
   - 原始来源是 `enc`
   - 包含 31 次 `round_function1` 调用和 1 次 `round_function2` 调用对应的 `PUSH` / `CALL`
2. `round_function1`
   - 包含 XOR、S-box、P-box、数组元素赋值等对应的三地址节点
3. `round_function2`
   - 包含最后一轮 addRoundKey 的简单过程表示

尤其要注意 `round_function1` 中这两类操作：

- `s<sbox_in>` 会在 TAC 中保留为 `BOXOP`
- `p<s_out>` 也会保留为 `BOXOP`

这正是 `Div2SetMILP` 后续识别 S-box / P-box 并生成约束的依据。

## 11. 进入 Div2SetMILP 之前，真正交出去的是什么

在进入 `Div2SetMILP` 前，最重要的输入有两类：

### 11.1 `allBox`

它提供：

- S-box / P-box 的原始整数表

`Div2SetMILP` 构造函数里也会复制这份数据：

```cpp
this->Box = allBox;
```

### 11.2 `procedureHs`

它提供：

- 主流程 `main`
- 轮函数
- 每个过程的 TAC 风格 `block`
- 返回值和参数的三地址表示

`Div2SetMILP::programGenModel()` 会从 `main` 开始，沿着 `CALL`、`PUSH`、`BOXOP` 等节点去建模。

## 12. 到这里为止，不同表示层分别是什么

为了后续理解方便，可以把 PRESENT 的预处理过程压缩成一张“表示层对照表”：

### 12.1 源码层

示例：

```cl
uint1[4] sbox_out = s<sbox_in>;
```

含义：

- 用户写的密码描述语言

### 12.2 AST 层

示例节点：

- `NVariableDeclaration`
- `NBoxOperation`
- `NForStatement`
- `NFunctionCall`

含义：

- 语法结构被树化
- 但还没有变成后端分析直接遍历的形式

### 12.3 解释器语义层

示例对象：

- `BoxValue`
- `ArrayValue`
- `InternalBinValue`
- `ProcCallValue`
- `ProcValue`

含义：

- AST 被“解释执行”
- 形成符号环境、过程语义、box 表和调用关系

### 12.4 TAC / ProcedureH 层

示例对象：

- `ProcedureH`
- `ThreeAddressNode`

含义：

- 语义对象被进一步压平
- 变成 `Div2SetMILP` 可以逐条遍历的中间表示

## 13. 用一句话概括 PRESENT.cl 的预处理轨迹

对于 PRESENT，EasyBC 在进入 `Div2SetMILP` 之前，经历的是这样一条链：

`PRESENT.cl 源码`
-> `programRoot(AST)`
-> `generateCode` 构造出 `allBox + ProcValue/Procedure + 符号环境`
-> `Transformer` 把 `ProcValue` 转成 `ProcedureH(ThreeAddressNode)`
-> `Div2SetMILP` 开始按 TAC 遍历主流程和轮函数

## 14. 对后续阅读代码最有帮助的三个抓手

如果后面要继续深入读 EasyBC 的预处理过程，最值得盯住的是这三个对象：

1. `allBox`
   - 负责把 `.cl` 中的 `sbox` / `pbox` 传给 division trail 和不等式生成模块
2. `ProcValue`
   - 负责承接解释器阶段的“过程语义”
3. `ProcedureH`
   - 负责承接进入 `Div2SetMILP` 前的 TAC 风格表示

如果把这三个对象之间的关系读通，`PRESENT.cl` 到 MILP 建模前的预处理主线就基本清楚了。
