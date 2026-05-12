# EasyBC 2-subset division-property 实验

本目录包含 `Div2SetMILP` 实现的 2-subset bit-based division property
MILP 积分搜索实验框架。

整个流程有意保持精简：C++ 代码只在 stderr 上输出结构化的 `[BENCH]`
日志行，本目录中的 Python 脚本负责编排运行、解析这些日志行，并生成 CSV /
LaTeX 表格。

## 目录结构

```
experiments/
├── README.md              (本文档)
├── configs/
│   ├── correctness.yaml   表 1 —— Xiang2016 结果复现实验矩阵
│   ├── round_sweep.yaml   表 2 —— 从第 1 轮开始的逐轮边界扫描
│   ├── perf.yaml          表 3 —— 同环境效率矩阵
│   └── reduction.yaml     表 4 —— S-box 约简方法消融实验
├── bench.py               矩阵运行器 —— 调用 ./EasyBC、解析 stderr、写入 CSV
├── parse_log.py           单日志检查工具 —— 输出 JSON 或人类可读摘要
├── make_tables.py         CSV → LaTeX 表格 + matplotlib 图
├── golden/                每个配置的 balanced-bit 标准答案 JSON（表 1 参考）
├── results/               CSV 输出 + 每次运行的日志（可由 gitignore 忽略）
└── tables/                渲染后的 LaTeX / PNG 输出
```

## 前置条件

- 构建 EasyBC（Gurobi 12.0.2 按仓库 CMakeLists 配置，默认使用 `build/`）：
  ```
  cmake --build build --target EasyBC -- -j
  ```
- Python 3.9+，并安装 PyYAML（`pip install pyyaml`）；只有绘图时才需要
  matplotlib。

## 快速开始

```bash
# （从仓库根目录执行）
cd experiments

# 1. 正确性冒烟测试（每个配置运行一次）
python bench.py --config configs/correctness.yaml

# 2. 检查某次具体运行的结果
python parse_log.py results/logs/PRESENT_R9_60_m1_t8_trial1_*.log --with-balanced

# 3. 填充 golden/ 后，渲染表 1
python make_tables.py --table correctness results/correctness_*.csv > tables/table1.tex

# 4. 逐轮边界扫描
python bench.py --config configs/round_sweep.yaml
python make_tables.py --table rounds results/round_sweep_*.csv > tables/table2.tex

# 5. 完整性能扫描（5 次重复 × 多个线程数）
python bench.py --config configs/perf.yaml
python make_tables.py --table perf results/perf_*.csv     > tables/table3.tex
python make_tables.py --plot scaling results/perf_*.csv

# 6. 约简方法消融实验
python bench.py --config configs/reduction.yaml
python make_tables.py --table reduction results/reduction_*.csv > tables/table4.tex
```

## 结构化日志格式

每次 EasyBC 运行会在 stderr 上输出以下一行或多行：

```
[BENCH] config cipher=<C> reduction=<M> rounds=<R> activebits=<A>
[BENCH] phase=trail      sbox=<S> elapsed_ms=<ms> n_trails=<N>
[BENCH] phase=ineq_gen   cipher=<C> sbox=<S> elapsed_ms=<ms> n_ineq=<N>
[BENCH] phase=reduce     cipher=<C> sbox=<S> method=<M> elapsed_ms=<ms> n_ineq_before=<B> n_ineq_after=<A>
[BENCH] phase=preprocess cipher=<C> elapsed_ms=<ms> n_ineq_loaded=<N>
[BENCH] phase=build      cipher=<C> rounds=<R> activebits=<A> elapsed_ms=<ms> n_xvars=<V> n_dvars=<D> block_size=<B>
[BENCH] phase=model_load cipher=<C> rounds=<R> activebits=<A> elapsed_ms=<ms> n_vars=<V> n_cons=<C>
[BENCH] phase=solve      cipher=<C> rounds=<R> activebits=<A> elapsed_ms=<ms> total_ms=<ms> gurobi_status=<S> distinguisher_found=<0|1> n_zero_coords=<N> block_size=<B> n_iter=<N> threads=<T>
```

如果一个算法包含多个 S-box（例如 LBlock 有 10 个），则一次运行中会出现多条
`trail` / `ineq_gen` / `reduce` 日志；`bench.py` 会将它们汇总到 CSV 的
`*_total` 列中，而 `parse_log.py` 会按 S-box 保留明细。

## balanced bits 语义

`Div2SetMILP` 的枚举过程会逐个找到可满足的单位输出向量，并把对应坐标
加入 `setZero` 后继续求解。因此 `result_*.txt` 中的中间 `xN=1` 行不是
平衡比特本身。最终平衡比特定义为：

```
balanced_bits = all_output_bits - setZero
```

新的结果文件会在末尾稳定输出：

```
Output bits: x...
Set zero: x...
Balanced bits: x...
```

`parse_log.py` 和 `make_tables.py` 都按这个定义解析结果。

## 方法说明（与实验计划一致）

- 每个实验单元重复 3-5 次；CSV 记录每次 trial；`make_tables.py` 计算中位数
  +（最小值，最大值）。
- Gurobi seed 默认值为 0（Gurobi 12 的默认值同样为 0；seed 方差实验作为
  单独的临时实验运行）。
- 单线程与 8 线程结果来自 `perf.yaml` 中的 `threads_sweep`；`bench.py` 会对
  每个列出的线程数运行每个实验单元。
- 超过 `timer_sec` 的配置会在 CSV 中被标记（`gurobi_status` ≠ 2 ≠ 3）。报告
  时用 † 标注。
- 硬件规格和 Gurobi 版本应写入论文 evaluation 部分的开头；请在本实验框架
  之外记录。

## 与外部实现对比

如需在同一台机器上与 `MILP_Division_Property-master` 或其他参考实现对比：

```bash
# 在参考实现中运行相同配置（该参考实现在本仓库之外）
# 将外部实现的时间手工填入论文 LaTeX 表格。
```

当前仓库不自动调用外部实现。EasyBC 的时间由 `perf.yaml` 生成；外部实现时间
在论文实验表中手工维护，并在表注中说明硬件、求解器版本和 timeout。

## CI 集成

`correctness.yaml` 矩阵也可以作为回归测试套件。完成一次干净复现后，填充
`golden/<key>.json`，并将以下命令加入 CI：

```bash
python bench.py --config configs/correctness.yaml --repeat 1
python make_tables.py --table correctness results/correctness_*.csv | grep -q '\\times' && exit 1
exit 0
```

（任何不匹配的 balanced-bit 集合都会在 LaTeX 中显示为 `\times`。）
