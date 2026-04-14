# Division Trails 复用 `greedy_sun` 的实现说明

## 1. 目标

本次修改的目标是：

- 在**不影响现有差分分析流程**的前提下
- 让 2 子集可分性分析复用 `lib/sbox/Reduction.cpp` 中已有的 `Red::greedy_sun`
- 消除 `DivTrailsModel.cpp` 中原本重复实现的一套贪心约简逻辑

核心思想是：

- `SboxDivTrails` 继续负责从 S-box 查找表生成 `Division Trails`
- `SboxM` 新增一条“从合法点集直接建模”的入口
- 将 `Division Trails` 作为 possible points 写入 `SboxM`
- 由 `SboxM` 自动生成：
  - `possPm_`
  - `impossPm_`
  - `sageIneqs_`
- 然后直接调用 `Red::greedy_sun(SboxM, ineqs)` 完成约简


## 2. 本次实际改动

### 2.1 `SboxM` 新增点集建模入口

在 `include/sbox/SboxModel.h` 与 `lib/sbox/SboxModel.cpp` 中新增：

```cpp
static SboxM fromPointSet(
    std::string name,
    std::vector<std::vector<int>> possPoints,
    std::string analysisTag,
    std::string outputRoot);
```

该入口当前只用于：

- `analysisTag == "division"`

其行为是：

1. 将 `possPoints` 转成 `possPm_`
2. 通过枚举全空间补集生成 `impossPm_`
3. 初始化 Sage 输入输出路径
4. 复用统一 helper 生成 Sage 不等式，得到 `sageIneqs_`

说明：

- `possPoints` 的输入格式为高位在前的 0/1 向量
- `SboxM` 内部统一转成字符串形式，例如 `"010011"`
- 这样可以继续兼容 `Check.cpp` 和 `Reduction.cpp` 中已有的二进制字符串 / 整数编码逻辑


### 2.2 `SboxM` 抽取公共 helper

为了同时支持差分分析和可分性分析，本次在 `SboxM` 中抽出了以下辅助函数：

- `pointToBinaryString(const std::vector<int>& point)`
- `ensureTrailingSlash(std::string path)`
- `initSagePaths(const std::string& basePath, int bitSize)`
- `buildImpossFromPossPoints()`
- `buildIneqsFromPossPoints(const std::vector<std::string>& possPoints)`

其中：

- `buildIneqsFromPossPoints(...)`
  - 负责写 Sage 输入脚本
  - 调用 `sage_ext()`
  - 解析生成的 `sageIneqs_`

这样就把“给定合法点集后生成凸包不等式”的能力统一到了 `SboxM` 中。


### 2.3 差分分析路径的处理

原有差分分析构造函数仍然是：

```cpp
SboxM(name, sbox, "differential", mode)
```

它的主流程没有改变本质语义，仍然会：

1. 生成 DDT
2. 提取 possible / impossible patterns
3. 生成 Sage 不等式
4. 生成 true table
5. 生成 poss 十六进制输出

需要特别说明的是：

- 构造函数里那一处显式的 `sage_ext()` 调用被删除了
- 但 `pattern_ext()` 结尾现在会调用 `buildIneqsFromPossPoints(possPm_)`
- 而这个 helper 内部最后仍然会执行 `sage_ext()`

因此：

- 差分分析**不会失去** Sage 不等式生成
- 只是把“显式调用位置”改成了“由 helper 统一触发”
- 同时避免将来出现重复调用 Sage 的问题


### 2.4 `DivTrailsModel` 改为复用 `greedy_sun`

原本 `lib/sbox/DivTrailsModel.cpp` 内部自己做了两件事：

1. 基于 `Division Trails` 写 Sage 脚本并解析不等式
2. 自己枚举补集点并执行贪心约简

本次改造后：

- 删除了 `DivTrailsModel` 内部这套重复实现
- 改为通过 `buildDivisionSboxModel()` 构造一个 division 专用 `SboxM`

对应逻辑如下：

```cpp
SboxM divisionSboxModel = SboxM::fromPointSet(
    sboxName_,
    divTrails_,
    "division",
    "../data/division/<cipher>/"
);
```

之后：

- `generateInequalities()`
  - 直接读取 `divisionSboxModel.get_sage_ineqs()`
- `reduceInequalities()`
  - 直接调用：

```cpp
Red::greedy_sun(divisionSboxModel, sageIneqs_)
```

这样 division 侧与差分侧就共享了同一套贪心约简核心。


### 2.5 构建系统更新

在 `CMakeLists.txt` 中补充了：

- `include/sbox/SboxDivTrails.h`
- `include/sbox/DivTrailsModel.h`
- `lib/sbox/SboxDivTrails.cpp`
- `lib/sbox/DivTrailsModel.cpp`

确保 division 相关模块能够被工程编译到。


## 3. `./EasyBC -div PRESENT` 的数据流

当执行：

```bash
./EasyBC -div PRESENT
```

整体流程如下：

1. `main.cpp` 进入 `DivTrailsMGR(...)`
2. 解析 `PRESENT` 对应的 `.cl` 文件
3. 遍历其中的所有 `sbox...`
4. 对每个 S-box：
   - `SboxDivTrails` 根据查找表生成 `Division Trails`
   - `DivTrailsModel` 接收 `Division Trails`
   - `DivTrailsModel::generateInequalities()`
     - 调 `SboxM::fromPointSet(...)`
     - 得到 `sageIneqs_`
   - `DivTrailsModel::reduceInequalities()`
     - 再构造同样的 division 专用 `SboxM`
     - 调 `Red::greedy_sun(...)`
     - 利用 `SboxM.get_imposs()` 中的补集点做贪心约简
5. 保存完整不等式与约简后不等式


## 4. `SboxM` 如何知道当前是可分性分析

`SboxM` 并不会直接读取命令行参数 `-div`。

它之所以知道当前是在做可分性分析，是因为调用方显式使用了：

```cpp
SboxM::fromPointSet(..., "division", ...)
```

也就是说：

- 差分分析走：

```cpp
SboxM(name, sbox, "differential", mode)
```

- 线性分析走：

```cpp
SboxM(name, sbox, "linear", mode)
```

- 可分性分析走：

```cpp
SboxM::fromPointSet(name, divTrails, "division", outputRoot)
```

因此三条入口是分开的，不会互相混淆。


## 5. 为什么这不会影响原有差分分析

本次实现遵循了以下兼容原则：

- 不修改现有差分构造函数签名
- 不修改 `Red::greedy_sun` 签名和内部贪心标准
- 不修改 `Check.cpp` 的 possible / impossible 判定逻辑
- 差分分析仍然使用原始 `SboxM(name, sbox, "differential", mode)` 路径
- division 只新增独立入口，不改写已有入口语义

差分路径中唯一容易误解的点是：

- 构造函数里删除了显式 `sage_ext()`

但实际并没有取消 Sage 不等式生成，因为：

- `pattern_ext()` 现在会在结尾调用 `buildIneqsFromPossPoints(possPm_)`
- 该 helper 内部最终仍会调用 `sage_ext()`

所以差分分析功能保持不变。


## 6. 当前实现的优点

- 消除了 `DivTrailsModel.cpp` 中重复的贪心约简代码
- 让 division 与 differential 共享同一套 `greedy_sun`
- 没有把 division 的生成逻辑硬塞进差分主流程
- 保持了 `main.cpp` 中 `DivTrailsMGR` 的调用方式不变
- 便于后续继续让 division 复用更多 `SboxM` / `Reduction` 能力


## 7. 当前实现的注意事项

- `DivTrailsModel::reduceInequalities()` 目前会再次构造一次 division 专用 `SboxM`
  - 这是为了最小侵入复用现有接口
  - 后续如果需要优化性能，可以把中间 `SboxM` 缓存为成员

- `Red::greedy_sun` 会按 `sboxM.get_path()` 输出额外的：
  - `reduction/...`
  - `statistics/...`
  文件
  这对 division 是正常的，只是比 `DivTrailsModel` 原始输出多一份内部统计文件

- 本次只复用了 `greedy_sun`
  - 没有把 division 接入 `sub_milp`
  - 没有接入 `convex_hull_tech`
  - 没有接入 `logic_cond`


## 8. 建议的后续工作

如果后续还想继续整理，可以考虑：

1. 将 `DivTrailsModel` 中构造的 division 专用 `SboxM` 缓存起来，避免重复生成 Sage 文件
2. 为 `pattern_ext()` 添加注释，明确“函数末尾会触发 Sage 不等式生成”
3. 如果后续 division 还要复用更多约简方法，可以进一步把 `Reduction` 层中依赖 `SboxM` 的公共逻辑再抽一层

