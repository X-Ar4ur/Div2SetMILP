#ifndef EASYBC_DIVTRAILSMODEL_H
#define EASYBC_DIVTRAILSMODEL_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "SboxModel.h"

class DivTrailsModel {

private:
    std::string cipherName_;
    std::string sboxName_;
    int sboxBitSize_;

    // 输入：来自 SboxDivTrails 的 Division Trails (possible 点集)
    std::vector<std::vector<int>> divTrails_;

    // SageMath 凸包生成的完整不等式 (H-表示)
    std::vector<std::vector<int>> sageIneqs_;

    // 约简后的不等式
    std::vector<std::vector<int>> reducedIneqs_;

    // 约简方法编号，用于 saveReducedInequalities 区分输出文件
    int reductionMethod_ = 1;

    SboxM buildDivisionSboxModel() const;

public:
    DivTrailsModel(std::string cipherName, std::string sboxName,
                   std::vector<std::vector<int>> divTrails, int sboxBitSize);

    /** 调用 SageMath 计算凸包，生成完整线性不等式 */
    void generateInequalities();

    /**
     * 约简不等式，method ∈ {1..7}：
     *   1 greedy_sun        (Sun 贪心)
     *   2 sub_milp          (Sub-MILP)
     *   3 convex_hull_tech  (Boura Alg1)
     *   4 logic_cond        (Boura Alg2)
     *   5 comb233           (Alg2+Alg3+Prop3)
     *   6 superball         (需要预置 superball 输入文件)
     *   7 external          (读取外部 cnf / Udovenko 结果)
     * 默认 1 以保持原行为。
     */
    void reduceInequalities(int method = 1);

    /** 保存完整不等式到文件 */
    void saveInequalities(const std::string& outputDir);

    /** 保存约简后不等式到文件 */
    void saveReducedInequalities(const std::string& outputDir);

    // 供后续 DivMILPcons / Div2SetMILP (Algorithm 3) 使用的接口
    std::vector<std::vector<int>> getSageIneqs() const { return sageIneqs_; }
    std::vector<std::vector<int>> getReducedIneqs() const { return reducedIneqs_; }
    int getSboxBitSize() const { return sboxBitSize_; }
    std::string getCipherName() const { return cipherName_; }
    std::string getSboxName() const { return sboxName_; }
};

#endif //EASYBC_DIVTRAILSMODEL_H
