# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概览

EasyBC 是基于 CL DSL（`.cl` 文件） + AST + TAC + MILP 建模的对称密码分析工具链
（Sun 等人原作，针对差分/线性）。本分支 `Linear-Layer-Division`（论文工作分支
`feature-division-extension`）扩展了 **bit-based 2-subset division property**
积分搜索。仓库的核心产物是供论文使用的 division-property 流水线；保留三个算法
边界（Alg 1 / 2 / 3）是后续重构的硬约束。

## 构建

CMake + Flex/Bison + Gurobi 12.0.2 + Z3 + jsoncpp。CMakeLists 中路径**硬编码为
Linux**（`/opt/gurobi1202/linux64`、`/usr/include/jsoncpp`、`z3/bin/libz3.so`），
Windows 开发主机通过 WSL 构建运行。本地不同机器需手改 `CMakeLists.txt:26-27`
（Gurobi）和 `:104`（z3 路径），或建立等价 symlink。

```bash
# 从仓库根目录
cmake -S . -B build
cmake --build build --target EasyBC -- -j
```

`EasyBC` 是唯一可执行目标——当前没有任何单元测试目标，`tests/` 目录已删除。
正确性靠 `experiments/configs/correctness.yaml` + `experiments/golden/*.json`
回归。

**二进制必须在 `build/` 目录下运行**：源码用相对路径 `../benchmarks/<set>/<cipher>.cl`
读取输入，`../data/division/<cipher>/` 写出结果。`experiments/bench.py` 会自动
设定 CWD。

## 四种调用模式（由 `main.cpp` 按 argc 分派）

1. **Division property** — `./EasyBC -div <CIPHER> [reduction] [rounds] [activebits] [timer N] [threads N]`
   - `reduction` ∈ 1..7（默认 1 = `greedy_sun`）：
     1 `greedy_sun` / 2 `sub_milp` / 3 `convex_hull_tech` (Boura Alg1) /
     4 `logic_cond` (Boura Alg2) / 5 `comb233` / 6 `superball` /
     7 external (Hellman / CNF)。方法 6/7 需要预置输入文件，常规运行不要用。
   - `activebits`：纯整数 `N` 走 cipher-specific 旧预设；`R<k>` / `L<m>R<k>` / `L<m>` /
     `hex:<mask>` 走 Feistel 风格右/左字（SIMON、Simeck）。解析在
     `Div2SetMILP::resolveActiveBitVars()`。
   - 只给 `rounds` 不给 `activebits` 时只完成 trails + inequalities，不触发
     `Div2SetMILP::MGR()`。
2. **MILP 差分/线性分析** — 8+ 个位置参数。见 `main.cpp` 的 `MILPMGR()` 注释块：
   `paramCount filePath {differential|linear} {singleKey|relatedKey} {w|b|d}
   {cryptanalysis|evaluation} reductionMethod`，其后是可选 `key value` 对
   （`startRound`、`endRound`、`timer`、`threadsNum`、`keySize`、`totalRoundNum`）。
   `w` 字宽，`b` 比特，`d` 扩展比特（带概率，触发 `sboxModelingMode=DC`）。
3. **S-box 建模** — 恰好 6 个位置参数：`sboxName sbox cryptanalysis mode redMd`。
   仅 differential 路径会触发 `Red::reduction`。
4. **无参兜底** — 读取 `../parametersMILPDemo.txt` 并按行数 5 / ≥7 重新派发到
   S-box 或 MILP 路径。`parametersMILPDemoSingleKey.txt`、`parametersSboxDemo.txt`
   只是参考模板，**不会被自动加载**。

## Division-property 流水线（论文核心）

`-div` 调用走以下四阶段，三个算法边界请勿打散：

1. **解析** `.cl` → AST（Flex/Bison，`lib/language/parser.y` + `lexer.l`）。
2. **`Interpreter::generateCode`** → 语义对象（`BoxValue`、`ProcValue`、`Procedure`），
   填充全局 `allBox` / `pboxM` / `Ffm`。
3. **`Transformer::transformProcedures`** → `ProcedureH` + `ThreeAddressNode`
   (TAC；`for` 全展开，`enc` 重命名为 `main`)。
4. **Algorithm 1 — `SboxDivTrails`**（`lib/sbox/SboxDivTrails.cpp`）：ANF + Möbius
   变换 → 每个输入对应最小输出 monomial 集 → division trails（LSB-first 2n 比特
   向量）。每个 S-box 一次，离线。
5. **Algorithm 2 — `DivTrailsModel`**（`lib/sbox/DivTrailsModel.cpp`）：构造
   `SboxM(..., analysisTag="division", ...)`，复用 `Red::reduction(method)`
   （`lib/sbox/Reduction.cpp`）。**不要重写 SageMath 凸包/贪心代码**——见
   `doc/division_greedy_sun_reuse_plan.md`。
   - **预计算缓存（8-bit S-box 必需）**：8-bit S-box（如 AES）的 16 维凸包用
     `Polyhedron().inequality_generator()` 不可行（会卡死）。若
     `benchmarks/precomputed_sbox_ineqs/<cipher>__<sbox>_Reduce_Inequalities.txt`
     存在，`main.cpp::DivTrailsMGR` 复制它到 `data/division/<cipher>/` 并**跳过整个
     SageMath 步骤**（cipher 无关，按文件存在触发）。缓存须是 EasyBC LSB-first
     `[in‖out]+b`、`≥−b` 格式。见 `doc/Linear-Layer_Phase_A_progress.md` §8。
6. **Algorithm 3 — `Div2SetMILP`**（`lib/division/Div2SetMILP.cpp` +
   `lib/division/DivMILPcons.cpp`）：TAC walker 把约束写成 `.lp`，再用 Gurobi
   迭代求解枚举平衡比特：
   - XOR：`x_out − x_in1 − x_in2 = 0`
   - COPY：`x_in − Σ x_copy_i = 0`
   - AND：`t ≥ u`、`t ≥ v`、`t ≤ u + v`
   - SBOX：每条约简后的不等式对 `[in‖out]` 内积 ≥ −b
   - **Iterative solver**：极小化输出比特之和；目标 ≤ 1 → 该比特平衡 → 把 UB
     钉到 0 后重解；循环至目标 > 1 或 infeasible。
   - **Lazy COPY-on-read (Phase 5)**：每次读取活变量都通过 `consumeCopy()`
     拆成 `x_live = x_a + x_b`；当死链尾被钉成 0 时退化为精确的 N 路 COPY。
     SIMON/Simeck 中 `l_input` 多次喂给 `p1/p2/p3` 的 fan-out 必须靠这套机制
     才能正确建模。

**平衡比特的语义**（`result_*.txt` 中间过程的 `xN=1` 行**不是**最终结果）：
`balanced_bits = all_output_bits − setZero`。结果文件末尾稳定输出三段
`Output bits / Set zero / Balanced bits`，`parse_log.py` 和 `make_tables.py`
都按此定义解析；任何输出格式调整都要同步它们。

## 差分/线性 MILP 模块（已有 EasyBC 机制）

- `DiffSWMILP` / `DiffSBMILP` — single-key 字宽 / 比特。
- `DiffRWMILP` / `DiffRBMILP` — related-key 字宽 / 比特。
- `LinearSBMILP` — single-key 比特；related-key 线性当前是占位空分支。
- 共享接口：`setGurobiTimer / setGurobiThreads / setStartRound / setEndRound /
  setKeySize / setTotalRoundNum / MGR()`；`cryptanalysis` 与 `evaluation`
  分支的差别是是否开启 `setSpeedUp1/2` 和 `setILP`。

## Cipher → `.cl` 映射

`-div` 通过 `setup::cryptPrimitiveMap[divCipherName]` +
`setup::cryptPrimitiveSetMap[divCipherName]` 解析到
`benchmarks/<set>/<RunName>.cl`。可用密码包括 SIMON 系列、Simeck 系列、PRESENT、
Rectangle、LBlock、TWINE、GIFT 等（详见 `experiments/configs/correctness.yaml`）。
要加新 cipher 需要同时在 `.cl` benchmark 和这两个 setup 表里登记。

AES 系列：`AES_GF4_toy`（GF(2⁴) 玩具，回归基线）、`AES_std`（正向标准 AES，GF(2⁸)；
8-bit S-box 凸包会卡 SageMath，**无**预计算缓存）、`AES_inv_std`（**逆 AES**，复现论文
4-round 的主线，配套预计算缓存，靠上节 skip-sage 机制绕开凸包）。逆 AES 的取舍与约定
（MSB-first、复用 mhgharieb repo 数据/算法）见 `doc/Linear-Layer_Phase_A_progress.md` §8
与 `doc/complex_linear_layer_division_plan.md` §9。

## Experiments 框架（`experiments/`）

C++ 在 **stderr** 上输出结构化 `[BENCH] phase=... key=val ...` 行；Python 端
按矩阵驱动 + 解析 + 生成 CSV/LaTeX。**改 `[BENCH]` 行格式必须同步更新
`parse_log.py` 与 `bench.py`**，否则评测全断。

```bash
cd experiments
python bench.py --config configs/correctness.yaml             # 表 1 复现
python bench.py --config configs/round_sweep.yaml             # 表 2 逐轮边界
python bench.py --config configs/perf.yaml \
       --out results/perf_present_rounds.csv                  # 表 3 性能（PRESENT R=1..10）
python bench.py --config configs/reduction.yaml               # 表 4 约简消融
python parse_log.py results/logs/<run>.log --with-balanced    # 单日志检查
python make_tables.py --table correctness results/correctness_*.csv > tables/table1.tex
python make_tables.py --table perf --cipher PRESENT results/perf_present_rounds.csv > tables/table3.tex
```

每次 `-div` 运行产生的 `[BENCH] phase=` 行：`trail` / `ineq_gen` / `reduce`
（每个 S-box 一行）+ `preprocess` / `build` / `model_load` / `solve`（整次运行
一行）。`bench.py` 把多 S-box 的同名 phase 汇总到 CSV 的 `*_total` 列。

**时间口径**（与 `experiments/README.md` 对齐）：
- 表 1 `T_EasyBC(s)`、表 2 `Time` 都来自 CSV `total_ms`，即
  `Div2SetMILP::iterativeSolver()` 内的 Gurobi 阶段总耗时（含 `.lp` 读入和
  迭代求解，**不**含 trail/ineq/reduce/build）。
- 表 3 `T_m(ms)` 来自 `build_ms`：`buildModel()` 的 TAC walk + `.lp` 写出。
- 表 3 `T_s(ms)` = `total_ms`，与表 1/2 同口径。

`experiments/golden/<cipher>_R<rounds>_<activebits>.json` 是 Xiang2016 /
Sun2017 / Eskandari 的参考平衡比特集，供 `correctness.yaml` 比对。

## 常用的单条 `-div` 命令（在 `build/` 内）

```bash
./EasyBC -div PRESENT 1 9 60                # PRESENT 9 轮，60 比特活动位
./EasyBC -div Simeck 1 20 R63               # 右字 activebits 规格
./EasyBC -div SIMON 1 17 63             # 显式左+右
./EasyBC -div LBlock 1 16 63 timer 7200 threads 8
```

输出落地：`../data/division/<cipher>/` 放 trails / inequalities，
`../data/division/<cipher>/milp/` 放 `.lp` 模型和 `result_*.txt`。`data/` 全部
gitignore，每次跑都会重建。

## 论文相关注意

- 用户用**简体中文**写作和沟通，除非显式要求英文。
- 三个值得强调的工程亮点：(a) Algorithm 2 复用 `SboxM` / `Red::reduction`，把
  差分和 division 统一到同一套不等式约简框架；(b) `Div2SetMILP` 的 lazy
  COPY-on-read；(c) 7 种可选 S-box 不等式约简方法。
- 不要把 Algorithm 2（S-box 离线建模）和 Algorithm 3（轮函数在线建模）混在
  一起描述。
- 相关设计文档：`doc/division_greedy_sun_reuse_plan.md`、
  `doc/present_preprocessing_pipeline.md`。

## 权限/会话设置

`.claude/settings.local.json` 已经存在，里面保存本仓库的工具权限白名单。
新增需要预先放行的命令时改这里，而不是 user-level settings。

## 工作规则
- 默认先阅读相关代码和文档，再开始修改。
- 修改前先说明影响范围。
- 只做和当前任务相关的最小改动。
- 提交前必须运行最小验证。
- 说明用中文，代码、命令、文件名保持英文。
- 不要修改无关文件。

## 实验原则
- 实验必须服务于明确假设或决策。
- 不要为了补齐表格而穷举低价值 ablation。
