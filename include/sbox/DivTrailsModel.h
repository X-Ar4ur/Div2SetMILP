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

    // 贪心约简后的不等式
    std::vector<std::vector<int>> reducedIneqs_;

    SboxM buildDivisionSboxModel() const;

public:
    DivTrailsModel(std::string cipherName, std::string sboxName,
                   std::vector<std::vector<int>> divTrails, int sboxBitSize);

    /** 调用 SageMath 计算凸包，生成完整线性不等式 */
    void generateInequalities();

    /** 贪心约简不等式 (Algorithm 1: InequalitySizeReduce) */
    void reduceInequalities();

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
