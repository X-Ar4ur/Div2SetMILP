# EasyBC BDPT 论文实验服务器复现

本文档对应 PRESENT 9 轮、数据复杂度 \(2^{63}\)、论文 Appendix F.9 的
3 子集 BDPT 实验。默认 `-div3` 行为保持不变；论文基线必须显式使用
`cross paper solver per-bit repro 1`。

## 1. 构建

```bash
export GUROBI_HOME=/home/xuws/gurobi1302/linux64
bash scripts/build_server.sh
```

构建脚本会先运行全部不依赖 Gurobi 许可证的测试，再生成
`build/EasyBC`。

## 2. 论文基线

```bash
bash scripts/reproduce_bdpt_present.sh \
  --timer 86400 \
  --threads 8
```

等价的 EasyBC 命令为：

```bash
build/EasyBC -div3 PRESENT 2 9 63 \
  cross paper \
  solver per-bit \
  sign 1 \
  repro 1 \
  timer 86400 \
  threads 8
```

脚本从 `build/` 启动 EasyBC，以满足项目已有的 `../benchmarks` 和
`../data` 相对路径约定。

结果目录：

```text
data/division/PRESENT/repro/subset3/9_63_paper_per-bit/
```

其中包含：

- `run.log`
- 全部 `M_t` 与 `M_L` LP
- `result.txt`
- `report.json`
- `report.csv`
- `report.md`
- `model_fingerprints.json`（每个 LP 的规范化 SHA-256、变量数、约束数和非零项数）

严格报告状态只有：

- `PASS`：全部模型和奇偶计数完成，NBB=28，位置逐位匹配 F.9，且 28 位均为 `sum=0`
- `MISMATCH`：求解完整，但结果与论文不同
- `INCOMPLETE`：超时、未知状态、solution pool 截断或模型检查失败
- `ERROR`：程序失败或没有生成结果

## 3. 四组合差分矩阵

```bash
bash scripts/reproduce_bdpt_matrix.sh \
  --timer 86400 \
  --threads 8
```

依次执行：

1. `paper/per-bit`
2. `paper/min-pin`
3. `exact/per-bit`
4. `exact/min-pin`

汇总文件位于：

```text
data/division/PRESENT/repro/subset3/matrix.json
data/division/PRESENT/repro/subset3/matrix.csv
data/division/PRESENT/repro/subset3/matrix.md
```

`matrix.json` 和 `matrix.md` 会逐个 \(M_t\) 报告相对
`paper/per-bit` 基线的 reachable-unit 集合差异。

## 4. 兼容性检查

原命令仍使用原默认值：

```bash
build/EasyBC -div3 PRESENT 2 9 63
```

对应：

```text
cross=exact
solver=min-pin
sign=1
repro=0
```

2 子集 `-div` 的参数解析和四个核心建模文件均未修改；可运行：

```bash
python3 tests/run_no_gurobi_tests.py
```

其中的哈希守卫会检查：

- `include/division/Div2SetMILP.h`
- `lib/division/Div2SetMILP.cpp`
- `include/division/DivMILPcons.h`
- `lib/division/DivMILPcons.cpp`
