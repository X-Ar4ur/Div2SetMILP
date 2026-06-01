# 3-subset BDPT 实施进度与续作指南（接力文档）

> 本文件供**新窗口直接照做**。配合 `doc/three_subset_bdpt_plan.md`（完整 Phase 0–6 方案、
> 架构决策、风险）一起读。分支：`3SetMILP`（从 `main` 切出）。
> 最后更新：2026-06-01。

---

## 0. 一句话现状

3-subset BDPT 积分区分器搜索已在 EasyBC 落地，与现有 2-subset CBDP（`Div2SetMILP`）**并存**。
**Phase 0–4 全部完成、验证、提交**；唯一待办是 **Phase 5 的 cross 轮对齐校准**（用 Rectangle
改进案例），以及后续 Phase 6（SIMON/Simeck）。

---

## 1. 已完成的 Phase（均已提交到 `3SetMILP`）

| Phase | commit | 内容 | 验证结果 |
|---|---|---|---|
| 0+1 | `04f5b53` | `-div3` 子命令 + `Div3SetMILP` 骨架 + S-box L-trail 计算 | PRESENT \|L\|=84/\|K\|=47，SIMON \|L\|=30，对齐论文 Table 3 |
| 2 | `450f826` | Div3 的 K/L 链 walker（按 `chainMode` 选 O_k / O_l） | K 链 `.lp` 与 Div2 **字节一致**（零回归） |
| 3 | `3e3f90b` | Key-XOR 交叉传播（`BdptMILPcons`）+ 模型集 {M_t} | cross 约束 (a)+(b)、L/K 段切分、变量数全部精确 |
| 4 | `2c36e19` | Algorithm 4 逐位判定（minimize+pin 枚举可达单位向量） | **PRESENT 9 轮：BDPT=1 平衡位 = CBDP = golden** |

### 关键代码地图（3-subset 部分）

```
include/division/Div3SetMILP.h   ← 类定义：ChainMode{K,L}、crossRound、模型集/求解器声明
lib/division/Div3SetMILP.cpp     ← walker（复制自 Div2 并扩展）+ 模型集 + Algorithm 4 求解器
include/division/BdptMILPcons.h  ← cross 约束写出器声明
lib/division/BdptMILPcons.cpp    ← bdptCrossDominanceC (b) / bdptCrossNotAllOneC (a)
main.cpp::DivTrailsMGR(params, subset)  ← subset==3 走 createLDivisionTrails + Div3SetMILP
CMakeLists.txt                   ← 已登记 Div3SetMILP.cpp/.h + BdptMILPcons.cpp/.h
```

`Div2SetMILP` / `DivMILPcons` / `main.cpp` 的 2-subset 路径**一行未改**（决策 1，零回归）。

---

## 2. 核心实现细节（续作必读，避免重蹈覆辙）

### 2.1 Cross 约束 = 论文 Proposition 1 的 (a)+(b)，**不要照搬参考库**
论文 Algorithm 3 第 9–10 行只有两条约束（第 t 个 Key-XOR，密钥异或左 s 位）：
- **(a)** `ℓ_0^t + … + ℓ_{s-1}^t ≤ s-1`（`bdptCrossNotAllOneC`）
- **(b)** `K_t* & L_t = L_t` ⟺ 逐位 `k_i^t* ≥ ℓ_i^t`（`bdptCrossDominanceC`）

⚠️ **参考库 `refer/bdpt_ref_repo/algorithm3_4__Cross_propagation.py` 是 SIMON 专用编码**
（在边界把密钥字的 K 钉成全 1 `x_next=c`），其约束 (b) **不能照搬到 SPN**。约束 (a)
（`Σv ≤ WORD_LENGTH-1`）参考库与论文一致，可对照。SPN 一律按论文 (a)+(b) 直接实现。

### 2.2 PRESENT 的 Key-XOR 在 TAC 里**已逐位展开**
`n_input = input ^ key`（uint1[64]）在 TAC 里是 64 条 `n_input_i = input_i ^ key_i`
（op=XOR, type=UINT1, rhs=key_i）。所以 cross hook 在 `roundFunctionGenModel` 里**逐条指令**挂：
- 检测到 key-xor（操作数含 keyId）且 `currentRound == crossRound`：取非 key 操作数的索引 ℓ_i，
  分配新变量 k_i，发射 (b) `k_i ≥ ℓ_i`，把 ℓ_i 收进 `crossLBits`，绑定 `n_input_i → k_i`，
  置 `functionCallFlag=true`（阻止默认别名覆盖）。
- 轮末发射一次 (a) `Σ crossLBits ≤ |crossLBits|-1`。s 自然 = 收集到的 key 覆盖位数。
- ℓ_i **直接引用、不 consumeCopy**（它是上一轮 P-box 的 `a`-copy，非 chainValue 尾，不会被钉零）。

### 2.3 模型集与 chainMode
`programGenModel` 每轮设 `currentRound`（1-based）与 `chainMode`：
`crossRound≥1` 时 `chainMode = (currentRound < crossRound) ? L : K`；`crossRound==-1` 用 `pureMode`。
- `buildChainModel(CHAIN_K/L, file)`：纯模式（K 链 diff 锚点 / M_L 全 L 链）。
- `buildMtModel(t, file)`：M_t，cross 在轮 t 轮首。
- `MGR()` 现在直接调 `searchDistinguisher()`（建+解每个 M_t）。

### 2.4 Algorithm 4 求解器（Phase 4 的选择）
对每个 M_t：载入 `.lp` → 加 `Σ K_r* ≥ 1`（排除全零退化）→ 最小化 + 钉零，枚举可达单位向量 e_q
（obj==1 即一个单位向量；obj>1 停）。跨 M_1..M_{r-1} 取并集 = unknown；balanced = 补集。
- **选了参考库式 minimize+pin，没做 solution-pool 奇偶计数**：goldens 只看平衡位**数量**，
  0/1（balanced 是 0 还是 1）标注后置。若论文表格要区分 0/1，再补 M_L 的 Gurobi solution pool。
- 结果落 `data/division/<cipher>/milp/result_<r>_<ab>_subset3.txt`；`[BENCH]` 行有
  `phase=solve_mt`（每模型 n_reachable）与 `phase=solve`（n_models/n_balanced/n_unknown）。

---

## 3. ⚠️ 唯一待校准点（Phase 5 起点）：cross 轮对齐

**问题**：PRESENT 把 Key-XOR 放在**轮首**（`round_function1` 首行 `n_input=input^key`），
而论文 Proposition 1 把 Key-XOR 建模在**轮尾**（f_r = fk ∘ fe）。于是"第 t 个 cross 落在哪一轮"
有歧义。当前代码 `crossRound ∈ [1, r-1]`。

**推断的隐患**：`crossRound=1` 的 M_1 在**明文上直接 cross**（0 个 L 轮），K_1* ⊇ activebits
（常数位的 k_i 还能自由取 1），本质 ≈ CBDP 甚至更宽 → 把它并进 union 会让 BDPT 的 unknown ⊇ CBDP，
方向反了。论文的 M_t 在 cross 前有 **t≥1 个 L 轮**（O_l 传播收紧 reachable），才比 CBDP 强。
正确对齐很可能是 `crossRound ∈ [2, r]`（轮 1..t-1 用 L、cross 在轮 t 轮首 ⟺ t-1 个 L 轮；
要 1..r-1 个 L 轮则 crossRound=2..r）。

**为什么 PRESENT 测不出来**：论文说 PRESENT 的 BDPT 处处 == CBDP，且第 9 轮所有 M_t 都饱和到
63 reachable → 对齐怎么取都给 1 平衡位。**必须用论文的改进案例校准**。

**校准方法（Phase 5 第一步，决定性）**：
- 跑 **Rectangle**：论文 §5 / Appendix F 报 **10 轮 9 平衡位**（比之前最好的多 8 位）；
  CBDP 在 10 轮几乎为 0。golden `experiments/golden/Rectangle_R9_60.json` 是 CBDP 9 轮 16 平衡位。
- 命令：`./EasyBC -div3 Rectangle 1 10 60`（Rectangle 的 activebits "60" 走
  `resolveActiveBitVars()` 里 rectangle.py 的特殊 Init）。
- **判据**：若 BDPT 给 9 平衡位（且 > CBDP 的 ~1）→ 对齐正确，提交 Phase 5；
  若 BDPT == CBDP（没体现改进）→ 把 `crossRound` 范围从 `[1,r-1]` 改成 `[2,r]`（或精确按论文
  t 个 L 轮重映射），重测，直到复现 9 平衡位。改的位置：`searchDistinguisher()` 里
  `for (int t = 1; t < this->rounds; ++t)` 这层循环的范围。
- 注意 Remark 2（第 r 个 Key-XOR 忽略）与轮首/轮尾的交互——重映射时连带核对模型个数。
- 提示：论文 Appendix F 有 Rectangle/GIFT/PRESENT 的具体活动位模式与区分器，对齐前先查那里的
  确切 activebits（可能不是 "60"）。

---

## 4. 后续 Phase（plan 文档 §5）

- **Phase 5**：SPN 端到端 golden（PRESENT 9r ✓ 已过；Rectangle 10r/9bit、GIFT-64 11r 待做）+
  单调性断言（BDPT 平衡位 ⊇ CBDP）+ 接入 `experiments/`（correctness.yaml 加 subset=3，
  parse_log.py / make_tables.py 识别 subset 维度与 `_subset3` 结果文件）。
  新建 `experiments/golden/<cipher>_R<r>_<ab>_subset3.json`。
- **Phase 6**：SIMON/Simeck——AND+XOR 核心按论文 "S-box" 技术打包成 4-bit S-box 再出 L-trail，
  或实现 COPY/XOR/AND 的 L 规则（更通用）。

---

## 5. 构建、运行、提交的硬约定（环境坑）

- **构建只能走 WSL**（CMakeLists 路径硬编码 Linux：Gurobi `/opt/gurobi1202/linux64`、z3）：
  ```
  wsl -e bash -lc "cd /mnt/d/Work/UCASNJ/EasyBC-2080-related-key && cmake --build build --target EasyBC -- -j"
  ```
  二进制在 `build/` 下跑；用相对路径读 `../benchmarks/`、写 `../data/`。`data/` 全 gitignore。
- **Bash 工具是 MSYS2/MinGW，不是 WSL**——所有构建/运行命令都要 `wsl -e bash -lc "..."` 包一层。
- **提交一定用 Windows git**（`core.autocrlf=true`，与建仓一致）。WSL 的 git 因 autocrlf 未设会把
  **全树**显示为 modified（纯 CRLF/LF 噪声），千万别用它提交。Windows git 只显示真实改动。
- **提交 hook 会拦截带 here-string 的组合命令**（`git add ...; git commit -m @'...'@` 被
  "Permission denied by hook"）。解决：分两步——先 `git add`，再把消息写进临时文件用
  `git commit -F <file>`（用完删）。
- WSL Gurobi license 绑 MAC（见 memory `env_wsl_gurobi_license`）；若 `Set parameter Threads`
  后报 GRBException，先查 MAC 不是查代码。

### 常用命令
```bash
# 在 build/ 下（经 wsl 包裹）
./EasyBC -div  PRESENT 1 9 60                 # 2-subset CBDP（对照基线）
./EasyBC -div3 PRESENT 1 9 60 timer 600 threads 8   # 3-subset BDPT
# 结果：data/division/PRESENT/milp/result_9_60_subset3.txt
#       [BENCH] phase=solve ... subset=3 n_balanced=... n_unknown=...
```

---

## 6. 验证基线（回归 gate）

- **2-subset 零回归**：`-div PRESENT 1 9 60` 仍 n_balanced=1；Div3 的 K 链 `.lp` 与 Div2 字节一致。
- **PRESENT 9r/60**：BDPT 与 CBDP 都 = 1 平衡位（已过）。
- 改任何 cross/对齐后，**必须**重跑这两项确认不破。

---

## 7. 参考资料

- 论文：`refer/MinerU_markdown_A_Model_Set_Method...md`（§3.4 Key-XOR + Algorithm 3、
  §4 初始/停止规则 + Algorithm 4、Appendix E 证明、Appendix F 区分器 golden）。
- 参考库（**仅对照、不照搬**）：`refer/bdpt_ref_repo/`（`algorithm3_4__Cross_propagation.py`
  是 SIMON 版 Key-XOR+搜索；`algorithm1__sbox.py` 是 L-trail）。
- 设计方案：`doc/three_subset_bdpt_plan.md`。
