#include "DivTrailsModel.h"
#include "Reduction.h"

DivTrailsModel::DivTrailsModel(std::string cipherName, std::string sboxName,
                               std::vector<std::vector<int>> divTrails, int sboxBitSize)
    : cipherName_(std::move(cipherName)), sboxName_(std::move(sboxName)),
      sboxBitSize_(sboxBitSize), divTrails_(std::move(divTrails)) {
}

SboxM DivTrailsModel::buildDivisionSboxModel() const {
    std::string outputRoot = std::string(DPATH) + "division/" + cipherName_ + "/";
    return SboxM::fromPointSet(sboxName_, divTrails_, "division", outputRoot);
}

void DivTrailsModel::generateInequalities() {
    SboxM divisionSboxModel = buildDivisionSboxModel();
    sageIneqs_ = divisionSboxModel.get_sage_ineqs();
    std::cout << "Inequalities generated: " << sageIneqs_.size()
              << " inequalities from SageMath convex hull." << std::endl;
}

void DivTrailsModel::reduceInequalities(int method) {
    reductionMethod_ = method;

    // 方法 1/2/3 以初始 sage 不等式为输入，需要先生成
    // 方法 4/5/6/7 不依赖初始 sage 不等式
    if (method == 1 || method == 2 || method == 3) {
        if (sageIneqs_.empty()) {
            generateInequalities();
        }
    }

    SboxM divisionSboxModel = buildDivisionSboxModel();
    reducedIneqs_ = Red::reduction(method, divisionSboxModel);

    std::cout << "Inequalities reduced (method " << method << "): "
              << sageIneqs_.size() << " -> " << reducedIneqs_.size() << std::endl;
}

void DivTrailsModel::saveInequalities(const std::string& outputDir) {
    std::string filepath = outputDir + sboxName_ + "_Inequalities.txt";
    std::ofstream file(filepath);
    if (!file) {
        std::cout << "Failed to write: " << filepath << std::endl;
        return;
    }

    for (const auto& ineq : sageIneqs_) {
        for (int k = 0; k < static_cast<int>(ineq.size()); ++k) {
            file << ineq[k];
            if (k < static_cast<int>(ineq.size()) - 1) file << "  ";
        }
        file << std::endl;
    }
    file.close();
    std::cout << "Full inequalities saved to: " << filepath << std::endl;
}

void DivTrailsModel::saveReducedInequalities(const std::string& outputDir) {
    if (reducedIneqs_.empty()) {
        std::cout << "No reduced inequalities to save." << std::endl;
        return;
    }

    // Archive per-method copy (useful for comparing reductions across methods)
    std::string archivePath = outputDir + sboxName_ + "_Reduce_M" +
                              std::to_string(reductionMethod_) + "_Inequalities.txt";
    // Active copy read by Div2SetMILP::preprocess(); overwritten by the most
    // recent reduction run (by design: each -div invocation picks one method).
    std::string activePath  = outputDir + sboxName_ + "_Reduce_Inequalities.txt";

    for (const std::string& filepath : {archivePath, activePath}) {
        std::ofstream file(filepath);
        if (!file) {
            std::cout << "Failed to write: " << filepath << std::endl;
            return;
        }
        for (const auto& ineq : reducedIneqs_) {
            for (int k = 0; k < static_cast<int>(ineq.size()); ++k) {
                file << ineq[k];
                if (k < static_cast<int>(ineq.size()) - 1) file << "  ";
            }
            file << std::endl;
        }
        file.close();
        std::cout << "Reduced inequalities saved to: " << filepath << std::endl;
    }
}
