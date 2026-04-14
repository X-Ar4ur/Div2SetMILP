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

void DivTrailsModel::reduceInequalities() {
    if (sageIneqs_.empty()) {
        generateInequalities();
    }

    SboxM divisionSboxModel = buildDivisionSboxModel();
    reducedIneqs_ = Red::greedy_sun(divisionSboxModel, sageIneqs_);

    std::cout << "Inequalities reduced: " << sageIneqs_.size()
              << " -> " << reducedIneqs_.size() << std::endl;
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

    std::string filepath = outputDir + sboxName_ + "_Reduce_Inequalities.txt";
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
