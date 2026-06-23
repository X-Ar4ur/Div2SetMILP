# EasyBC 3 子集 BDPT 论文实验结果复现设计

日期：2026-06-23

状态：设计已由用户批准，等待书面审阅

目标论文：*A Model Set Method to Search Integral Distinguishers Based on Division Property for Block Ciphers*

## 1. 背景

EasyBC 当前已经具备独立的 `-div3` 路径，能够生成 S-box 的 K-trails 与
L-trails、构造模型集合 \(P=\{M_1,\ldots,M_{r-1}\}\)、搜索输出单位向量并使用
\(M_L\) 标记 balanced bit 的和为 0 或 1。

针对命令：

```bash
./EasyBC -div3 PRESENT 2 9 63
```

已有代码和实验产物表明：

- PRESENT S-box 的 K-trails、L-trails 及 method 2 约简规模与论文 Table 3
  一致；
- 当前未完成的 PRESENT 9 轮结果在已求解模型上已经得到 28 个候选
  balanced bits；
- 这些候选位置经过 EasyBC/PRESENT 的输出位序映射后与论文 Appendix F.9
  一致；
- 现有任务仍未完成全部 \(M_1,\ldots,M_8\) 及 \(M_L\)，因此不能把部分结果
  当作正式复现；
- 现有 Rectangle 10 轮产物尚未复现论文报告的 9 个 balanced bits；
- 论文公开源码不完整，且 Algorithm 3/4 的公开实现主要面向 SIMON，不能直接
  作为 PRESENT、RECTANGLE、GIFT-64 的完整基准。

因此，本设计将“论文结果复现”定义为可审计、可重复、可区分模型差异与求解
差异的工程任务，而不是只追求得到相同的 NBB 数字。

## 2. 目标与验收优先级

### 2.1 第一阶段目标：结果复现

第一阶段只以结果正确性为主，不以论文的 4.6 分钟为硬性要求。

PRESENT 9 轮、数据复杂度 \(2^{63}\) 的严格验收条件为：

1. 完整构造并求解全部 \(M_1,\ldots,M_8\)；
2. 完整执行 \(M_L\) 的 balanced bit 符号判定；
3. 最终 NBB 必须为 28；
4. 28 个 balanced bit 的位置必须在显式位序转换后与论文 Appendix F.9
   完全一致；
5. 28 个 balanced bit 必须全部被可靠判定为 `sum=0`；
6. 不允许使用“前几个模型的并集已经得到 28 位”作为提前成功条件；
7. 任一求解超时、状态未定、solution pool 截断或模型检查失败，论文复现运行
   均标记为“未完成”，不能标记为成功。

PRESENT 完成后，使用相同框架依次复现：

1. RECTANGLE 10 轮、\(2^{63}\)、NBB=9；
2. GIFT-64 论文报告的轮数、输入模式和 NBB；
3. 论文 Table 1 中其他适合当前 EasyBC 3 子集通用路径的实验。

### 2.2 第二阶段目标：性能复现

只有结果复现通过后，才进入性能优化阶段。性能阶段将：

- 明确计时是否包含 trails、凸包、不等式约简、LP 构造、全部 \(M_t\) 求解和
  \(M_L\) 奇偶计数；
- 固定 CPU、Gurobi 版本、线程数、参数、随机种子和超时；
- 分别报告 wall time 与 solver time；
- 将论文的 4.6 分钟视为待验证目标，而不是结果正确性的前提。

## 3. 硬性隔离边界

本任务不得改变 2 子集 MILP 的行为。

### 3.1 禁止修改

以下文件和接口不进入本任务的修改范围：

- `include/division/Div2SetMILP.h`
- `lib/division/Div2SetMILP.cpp`
- `include/division/DivMILPcons.h`
- `lib/division/DivMILPcons.cpp`
- `-div` 的参数语义、默认值、输出格式与结果语义

如果实现过程中发现必须修改上述路径才能继续，应停止实施并重新提交设计审议，
不能自行扩大范围。

### 3.2 允许修改

修改限定在：

- `include/division/Div3SetMILP.h`
- `lib/division/Div3SetMILP.cpp`
- `include/division/BdptMILPcons.h`
- `lib/division/BdptMILPcons.cpp`
- `main.cpp` 中仅属于 `-div3` 的参数解析分支
- 新增的 BDPT 专用测试、实验配置、golden result 和报告生成代码

### 3.3 2 子集零回归要求

实施前保存代表性 `-div` 基线。实施后对相同命令比较：

- division trails；
- 完整与约简不等式；
- LP 的规范化内容；
- balanced bit 结果；
- `[BENCH]` 关键字段和现有解析兼容性。

除不稳定元数据（绝对路径、时间、日志顺序等）外，任何语义差异均视为回归失败。

## 4. 总体方案

采用“论文原样模型 + 当前精确模型”的 BDPT 双轨实现，并将交叉建模策略和单位
向量搜索策略拆成两个互相独立的配置维度。

### 4.1 交叉约束模式

#### `cross=paper`

按论文 Algorithm 3 / Proposition 1 构造 Key-XOR cross：

- key 覆盖位置上的 L 向量不能全为 1；
- \(K_t^*\) 逐位支配 \(L_t\)；
- 不额外加入 one-hot selector；
- 该模式是论文复现基线。

#### `cross=exact`

保留当前 `bdptCrossExactOneFlipC()`：

- 为每个 key 覆盖位置引入 selector；
- 恰好选择一个在 \(L_t\) 中为 0 的位置；
- \(K_t^*\) 等于 \(L_t\) 将该位置翻转为 1；
- 该模式代表 EasyBC 当前更严格的工程实现。

两种模式必须使用相同的 L-chain、K-chain、初始 BDPT、输出位序和停止规则。

### 4.2 单位向量搜索模式

#### `solver=per-bit`

严格对应论文 Algorithm 4 的判定方式：

- 对每个 \(M_t\) 和输出坐标 \(q\)，固定完整输出
  \(K_r^*=e_q\)；
- 执行可行性判定；
- 可行则将 \(q\) 加入 unknown 集；
- 每个坐标必须得到明确的 `FEASIBLE` 或 `INFEASIBLE` 结论。

该模式是论文复现基线。

#### `solver=min-pin`

保留当前 minimize-and-pin 策略：

- 最小化 \(K_r^*\) 的输出重量；
- 每找到一个重量为 1 的解，将相应输出坐标上界固定为 0；
- 重复求解，直到证明最优值大于等于 2 或模型不可行；
- 已由更早 \(M_t\) 证明 unknown 的坐标可在后续模型中排除。

该模式用于结果等价验证和后续性能优化。

### 4.3 四组合差分矩阵

复现框架支持：

| 组合 | cross | solver | 用途 |
|---|---|---|---|
| A | paper | per-bit | 论文原样复现基线 |
| B | paper | min-pin | 验证求解优化是否保持论文模型结果 |
| C | exact | per-bit | 单独观察交叉约束收紧的影响 |
| D | exact | min-pin | 当前 EasyBC 默认路径 |

对同一输入，报告每个 \(M_t\) 的单位向量集合差异。不得只比较最终 NBB，因为不同
模型可能偶然得到相同的 NBB。

## 5. 命令行与默认行为

### 5.1 论文复现命令

```bash
./EasyBC -div3 PRESENT 2 9 63 \
  cross paper \
  solver per-bit \
  sign 1 \
  repro 1
```

### 5.2 现有默认命令

```bash
./EasyBC -div3 PRESENT 2 9 63
```

未提供新参数时保持当前默认行为：

```text
cross=exact
solver=min-pin
sign=1
repro=0
```

因此，新增功能不会悄悄改变现有 `-div3` 命令的模型语义。

### 5.3 参数规则

- `cross` 仅接受 `paper` 或 `exact`；
- `solver` 仅接受 `per-bit` 或 `min-pin`；
- `repro` 仅接受 `0` 或 `1`；
- 参数错误必须在构造模型前失败，并打印完整的 `-div3` 用法；
- `-div` 分支不识别也不读取这些参数。

## 6. 组件设计

### 6.1 `Div3SetMILP` 配置

新增 BDPT 专用枚举和状态：

```cpp
enum class BdptCrossMode { Paper, Exact };
enum class BdptUnitSearchMode { PerBit, MinPin };
```

并提供只作用于 `Div3SetMILP` 的 setter。`repro` 控制严格失败语义和独立实验输出，
不改变数学模型本身。

### 6.2 `BdptMILPcons` 约束入口

保留：

```cpp
bdptCrossExactOneFlipC(...)
```

新增论文模式入口，例如：

```cpp
bdptCrossPaperC(...)
```

论文入口只组合论文明确给出的：

- `bdptCrossNotAllOneC(...)`
- 每个位置的 `bdptCrossDominanceC(...)`

不得为了提高求解速度在 `paper` 模式中静默加入 selector、重量增量等更强约束。
任何额外有效不等式只能作为后续独立实验模式加入，不能混入论文基线。

### 6.3 单位向量求解接口

将当前 `solveMtReachableCoords()` 拆为清晰的 BDPT 专用策略：

```cpp
solveMtReachableCoordsPerBit(...)
solveMtReachableCoordsMinPin(...)
```

统一返回结构化结果，而不是只有坐标集合。结果至少包含：

- reachable coordinates；
- 每个坐标或每轮迭代的 Gurobi status；
- 是否完整证明；
- 求解次数；
- solver time；
- 超时或异常原因。

调用者根据 `BdptUnitSearchMode` 选择策略。

### 6.4 \(M_L\) 奇偶判定

保留 \(M_L\) 的独立模型，但在 `repro=1` 时使用严格语义：

- 必须完成全解计数或得到可证明等价的奇偶结果；
- `TIME_LIMIT`、pool cap、模型 sanity check 失败均返回
  `INCOMPLETE`；
- `INCOMPLETE` 不能转换成 `b` 后继续宣称论文复现成功；
- 普通 `repro=0` 路径继续保留当前安全的 `b` 回退。

第一阶段不假设 Gurobi solution pool 一定能在合理时间完成。若它成为正式阻塞，
后续设计可引入不改变结果的 parity-specific 算法，但必须与小规模穷举结果交叉验证。

### 6.5 位序映射

论文输出字符串与 EasyBC 内部坐标不能凭观察手工比较。新增显式的 cipher-specific
展示映射，仅用于报告和 golden comparison：

- 内部输出坐标；
- P-box 前后坐标；
- nibble 内 bit 顺序；
- 论文字符串位置。

数学模型继续使用 EasyBC 现有内部位序，映射层不得进入约束构造。

## 7. 运行数据流

一次严格复现运行按以下顺序执行：

1. 解析 `-div3` 与 BDPT 专用参数；
2. 生成并验证 K/L division trails；
3. 生成、约简并验证 K/L 不等式可行域；
4. 对 \(t=1,\ldots,r-1\)：
   - 构造 \(M_t\)；
   - 记录模型指纹和规模；
   - 按指定 solver 求单位向量集合；
   - 严格检查求解是否完成；
5. 合并所有 \(M_t\) 的 unknown 坐标；
6. 取补集得到 NBB；
7. 构造 \(M_L\)，对所有 determined 坐标完成奇偶判定；
8. 显式转换成论文输出位序；
9. 与 golden result 逐位比较；
10. 输出机器可读记录和中文复现报告。

## 8. 输出与可审计性

### 8.1 输出隔离

严格复现产物写入独立目录，不覆盖普通 `-div3` 结果，例如：

```text
data/division/PRESENT/repro/subset3/
```

每次运行使用包含配置摘要的 run id，保存：

- 所有 \(M_t\) 与 \(M_L\) LP；
- 每个模型的求解摘要；
- 最终结果；
- 环境信息；
- JSON、CSV 和 Markdown 报告。

### 8.2 模型指纹

为每个 LP 生成规范化指纹，至少忽略：

- 文件路径；
- 注释；
- 非语义空白。

保留并记录：

- 变量数量；
- 约束数量；
- 非零系数数量；
- 二进制变量数量；
- 规范化内容哈希。

### 8.3 复现状态

最终状态只允许：

- `PASS`：所有严格条件满足；
- `MISMATCH`：求解完整，但与论文结果不一致；
- `INCOMPLETE`：存在超时、未知状态、计数截断或外部环境阻塞；
- `ERROR`：输入、模型或程序错误。

不得将 `INCOMPLETE` 报告成“没有区分器”或“复现成功”。

## 9. 验证设计

### 9.1 2 子集隔离回归

选择覆盖 SPN 与 Feistel 的代表性 `-div` 命令。建议至少包含：

```bash
./EasyBC -div PRESENT 1 9 60
./EasyBC -div Rectangle 1 9 63
./EasyBC -div SIMON32 1 14 31
```

如果当前环境无法完成全部求解，至少比较可生成的 trails、不等式和 LP；完整结果
回归在有效 Gurobi 环境恢复后补齐，未补齐前不得合并实现。

### 9.2 S-box 级验证

- PRESENT K-trails 必须为 47；
- PRESENT L-trails 必须为 84；
- method 2 约简后 K/L 不等式分别为 8/20；
- 约简前后在 \(\{0,1\}^{8}\) 上的可行点集合必须一致；
- PRESENT L-trails 在显式位序映射后与论文 Table 5 逐项一致。

### 9.3 Cross 约束穷举

对小位宽和 PRESENT 的 4-bit 局部输入穷举：

- `paper` 可行域必须等于论文两组约束定义的集合；
- `exact` 可行域必须等于“从 L 的一个 0 位恰好翻转为 1”的集合；
- 报告 `paper - exact` 与 `exact - paper`；
- 不预设两者相等。

### 9.4 Solver 等价验证

在相同 LP 上：

- 小轮数 PRESENT 的 `per-bit` 与 `min-pin` 必须返回相同单位向量集合；
- 对每个有差异的坐标保存可复查模型；
- 只有小规模等价验证通过后，才运行 9 轮完整矩阵。

### 9.5 PRESENT 端到端验证

四种组合都必须执行全部 \(M_1,\ldots,M_8\)。报告：

- 每个 \(M_t\) 的 reachable unit coordinates；
- 四组合间的集合差；
- 最终 unknown、balanced、sum=0、sum=1；
- 内部坐标和论文坐标；
- 与 Appendix F.9 的逐位 diff；
- 模型规模、求解次数和耗时。

论文基线 A 必须满足第 2.1 节的所有严格条件。

## 10. 实施顺序

实施拆成可独立验证的步骤，每一步单独提交：

1. 新增 2 子集基线采集与规范化比较脚本，不改建模代码；
2. 新增 BDPT 配置枚举、默认值和 `-div3` 参数解析；
3. 新增 `cross=paper`，并完成局部可行域穷举测试；
4. 将单位向量搜索拆为 `per-bit` 与 `min-pin` 两个策略；
5. 新增结构化求解结果和严格 `repro` 状态；
6. 新增论文位序映射及 PRESENT golden result；
7. 新增四组合小轮数差分测试；
8. 完成 PRESENT 9 轮全部 \(M_t\)；
9. 完成 \(M_L\) 的 28 位 `sum=0` 验证；
10. 生成 PRESENT 正式复现报告；
11. 用同一框架复现 RECTANGLE；
12. 用同一框架复现 GIFT-64；
13. 结果全部稳定后，另立性能优化设计。

任何一步出现数学结果变化，都在该步骤停止，不把问题混入后续优化。

## 11. 风险与处理原则

### 11.1 论文约束可能是松弛表示

`paper` 与 `exact` 的可行域可能不同。此时不能把差异直接称为 bug，应比较：

- 每个 \(M_t\) 的单位向量集合；
- 最终区分器结果；
- 论文声称的“少量精度损失”是否能解释差异。

### 11.2 \(M_L\) 全解奇偶计数可能不可扩展

如果 solution pool 无法完成：

- 运行状态为 `INCOMPLETE`；
- 不降低验收标准；
- 另行设计可证明正确的奇偶求解方法；
- 在小规模模型上与完全枚举交叉验证后才能替换。

### 11.3 论文时间不可直接比较

论文没有提供足够完整的环境与时间口径。正式报告必须明确当前实验环境，并把：

- 结果复现；
- 数学模型对齐；
- 性能接近

作为三个独立结论。

### 11.4 Gurobi 环境阻塞

当前本机检测到的 Gurobi 许可证已于 2026-06-10 到期。在许可证恢复前可以完成：

- 配置与代码结构；
- LP 构造；
- 布尔可行域穷举测试；
- golden result 和报告框架；
- 不依赖求解器的 2 子集回归。

完整 PRESENT/RECTANGLE/GIFT 求解必须在有效许可证环境中执行。

## 12. 完成定义

第一阶段只有在以下条件全部满足时才完成：

- 2 子集零回归通过；
- `paper/per-bit` 完整求解 PRESENT 全部 \(M_t\) 与 \(M_L\)；
- NBB=28；
- 28 个位置与论文 F.9 一致；
- 28 位全部可靠标记为 `sum=0`；
- 四组合差异有完整记录；
- 结果可由一条命令和固定配置重新生成；
- JSON、CSV、Markdown 报告包含环境、模型指纹、逐模型集合和最终 diff；
- 未使用超时回退或部分结果冒充成功。

完成这些条件后，才开始第二阶段的性能优化与 4.6 分钟目标分析。
