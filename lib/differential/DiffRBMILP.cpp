//
// Created by Septi on 6/9/2023.
//

#include "differential/DiffRBMILP.h"

extern std::map<std::string, std::vector<int>> allBox;
extern std::map<std::string, std::vector<int>> pboxM;
extern std::map<std::string, int> pboxMSize;
extern std::map<std::string, std::vector<int>> Ffm;
extern std::string cipherName;


DiffRBMILP::DiffRBMILP(std::vector<ProcedureHPtr> procedureHs, std::string target, std::string mode, int redMode)
        : procedureHs(std::move(procedureHs)), target(move(target)), mode(move(mode)), redMode(redMode) {
    this->Box = allBox;
    this->cipherName = ::cipherName;
}


void DiffRBMILP::MGR() {
    // initial the information of all boxes,
    // e.g., the weighted of S-boxes used in "DC" mode, the modeling inequalities of S-boxes, the branch numbers etc.
    preprocess(pboxM, Ffm);

    this->finalResults.clear();
    this->finalClockTime.clear();
    this->finalTimeTime.clear();

    // Initial directories
    this->pathPrefix += target + "/";
    this->modelPath = this->pathPrefix + "models/" + this->pathSuffix;
    this->resultsPath = this->pathPrefix + "results/" + this->pathSuffix;
    this->diffCharacteristicPath = this->pathPrefix + "differentialCharacteristics/" + this->pathSuffix;
    system(("mkdir -p " + this->modelPath).c_str());
    system(("mkdir -p " + this->resultsPath).c_str());
    system(("mkdir -p " + this->diffCharacteristicPath).c_str());
    this->finalResultsPath = this->resultsPath + this->cipherName + "_FinalResults.txt";
    system(("touch " + this->finalResultsPath).c_str());

    for (int i = this->startRound; i <= this->endRound; ++i) {
        // 如果证明block cipher是安全的，就停止, 只有在target == "evaluation" 时，才可以被改变
        if (this->securityFlag)
            break;

        this->currentRound = i;
        std::cout << "\n **************** CURRENT MAX ROUOND : " << this->currentRound << "  **************** \n" << std::endl;
        // Initial path
        this->modelPath = this->pathPrefix + "models/" + this->pathSuffix + std::to_string(this->currentRound) + "_round_model.lp";
        this->resultsPath = this->pathPrefix + "results/" + this->pathSuffix + std::to_string(this->currentRound) + "_round_results.txt";
        system(("touch " + this->modelPath).c_str());
        system(("touch " + this->resultsPath).c_str());
        // Differential characteristic path
        this->diffCharacteristicPath = this->pathPrefix + "differentialCharacteristics/" + this->pathSuffix + std::to_string(this->currentRound) + "_round_differential_characteristic.txt";
        system(("touch " + this->diffCharacteristicPath).c_str());

        system(("rm " + this->modelPath).c_str());
        if (this->target == "cryptanalysis")
            cryptanalysis();
        else if (this->target == "evaluation")
            evaluation();
        else
            assert(false);

        /*
         * 因为我们用sat问题求解进行evaluation时，会有很多不可解的模型，产生很多不必要的时间开销，
         * 因此我们需要也统计一下，每轮的所有时间，即不可解模型的求解时间也加上。
         * */
        // 下面两个时间也记录不可解模型的求解时间
        vector<double> finalTotalRoundClockTime;
        vector<double> finalTotalRoundTimeTime;
        clock_t startTime, endTime;
        startTime = clock();
        time_t star_time = 0, end_time;
        star_time = time(NULL);
        solver();
        endTime = clock();
        end_time = time(NULL);
        double clockTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
        double timeTime = difftime( end_time, star_time);
        finalTotalRoundClockTime.push_back(clockTime);
        finalTotalRoundTimeTime.push_back(timeTime);

        // Initial for next round
        this->tanNameMxIndex.clear();
        this->rtnMxIndex.clear();
        this->rtnIdxSave.clear();
        this->kschdNameMxIndex.clear();
        this->kschdRtnIdxSave.clear();
        this->xCounter = 1;
        this->dCounter = 1;
        this->ACounter = 1;
        this->PCounter = 1;
        this->fCounter = 1;
        this->yCounter = 1;
        this->lastRoundACounter = 1;
        this->lastRoundPCounter = 1;
        this->allRoundACounters.clear();
        this->allRoundPCounters.clear();
        this->differentialCharacteristic.clear();
        // Save the current final results
        std::ofstream fResults(this->finalResultsPath, std::ios::trunc);
        if (!fResults){
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            if (this->target == "cryptanalysis") {
                fResults << " ***************** FINAL RESULTS ***************** \n\n";
                for (int j = 0; j < this->finalResults.size(); ++j) {
                    fResults << "Results Of " << j + 1 << " Rounds : \n";
                    fResults << "\tObj : " << this->finalResults[j] << "\n";
                    fResults << "\tclockTime : " << this->finalClockTime[j] << "\n";
                    fResults << "\ttimeTime : " << this->finalTimeTime[j] << "\n\n";
                }
                fResults << " ***************** FINAL RESULTS ***************** \n";
            } else if (this->target == "evaluation") {
                fResults << "-------------------------- FINAL RESULTS -------------------------- \n\n";
                for (int j = 0; j <= this->finalResults.size(); ++j) {
                    if (j == this->securityRoundNumBound - 1) {
                        fResults << "\n****************************************************************\n";
                        fResults << "the block cipher is security enough when the round number is : " << j + 1 << "\n";
                        fResults << "****************************************************************\n\n\n\n";

                        fResults << "***************** Results Of " << j + 1 << " Rounds *****************\n";
                        // fResults << "\tnumber of active sboxes for security bounding : " << this->finalResults[j] << "\n";
                        fResults << "\tnumber of target active sboxes for security bounding : " << this->targetNdiffOrPr << "\n";
                        fResults << "\tclockTime : " << this->finalClockTime[j] << "\n";
                        fResults << "\ttimeTime : " << this->finalTimeTime[j] << "\n\n";
                        // 加上不可解模型求解时间的所有时间
                        fResults << "\ttotalRoundClockTime : " << finalTotalRoundClockTime[j] << "\n";
                        fResults << "\ttotalRoundTimeTime : " << finalTotalRoundTimeTime[j] << "\n\n\n";
                    } else {
                        fResults << "***************** Results Of " << j + 1 << " Rounds *****************\n";
                        fResults << "\tnumber of active sboxes : " << this->finalResults[j] << "\n";
                        fResults << "\tclockTime : " << this->finalClockTime[j] << "\n";
                        fResults << "\ttimeTime : " << this->finalTimeTime[j] << "\n\n";
                        // 加上不可解模型求解时间的所有时间
                        fResults << "\ttotalRoundClockTime : " << finalTotalRoundClockTime[j] << "\n";
                        fResults << "\ttotalRoundTimeTime : " << finalTotalRoundTimeTime[j] << "\n\n\n";
                    }
                }
                fResults << " -------------------------- FINAL RESULTS -------------------------- \n";
            }
            fResults.close();
        }
    }
    // Print final results
    if (target == "cryptanalysis") {
        std::cout << "\n ***************** FINAL RESULTS ***************** " << std::endl;
        for (int i = 0; i < this->finalResults.size(); ++i) {
            std::cout << "Results Of " << i + 1 << " Rounds : " << std::endl;
            std::cout << "\tObj : " << this->finalResults[i] << std::endl;
            std::cout << "\tclockTime : " << this->finalClockTime[i] << std::endl;
            std::cout << "\ttimeTime : " << this->finalTimeTime[i] << std::endl << std::endl;
        }
        std::cout << " ***************** FINAL RESULTS ***************** \n" << std::endl;
    } else if (target == "evaluation") {
        std::cout << "\n ***************** FINAL RESULTS ***************** " << std::endl;
        for (int i = 0; i < this->finalResults.size(); ++i) {
            if (i == this->securityRoundNumBound - 1) {
                std::cout << "\n\n****************************************************************" << std::endl;
                std::cout << "the block cipher is security enough when the round number is : " << i + 1 << std::endl;
                std::cout << "****************************************************************\n\n\n" << std::endl;

                std::cout << "Results Of " << i + 1 << " Rounds : " << std::endl;
                // std::cout << "\tnumber of active sboxes for security bounding : " << this->finalResults[i] << std::endl;
                std::cout << "\tnumber of target active sboxes for security bounding : " << this->targetNdiffOrPr << "\n";
                std::cout << "\tclockTime : " << this->finalClockTime[i] << std::endl;
                std::cout << "\ttimeTime : " << this->finalTimeTime[i] << std::endl << std::endl;
            } else {
                std::cout << "Results Of " << i + 1 << " Rounds : " << std::endl;
                std::cout << "\tnumber of active sboxes : " << this->finalResults[i] << std::endl;
                std::cout << "\tclockTime : " << this->finalClockTime[i] << std::endl;
                std::cout << "\ttimeTime : " << this->finalTimeTime[i] << std::endl << std::endl;
            }
        }
        std::cout << " ***************** FINAL RESULTS ***************** \n" << std::endl;
    }
}


void DiffRBMILP::cryptanalysis() {
    programGenModel();

    std::ifstream file;
    file.open(this->modelPath);
    std::string model,line;
    while (getline(file, line)) {model += line + "\n";}
    file.close();

    if (model != "") {
        std::ofstream target(this->modelPath, std::ios::trunc);
        if (!target) {
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            target << "Minimize\n";
            if (this->mode == "AS") {
                for (int i = 1; i < this->ACounter; ++i) {
                    target << "A" << i;
                    if (i != this->ACounter - 1) target << " + ";
                }
            } else if (this->mode == "DC") {
                if (!this->sboxExtWeighted.empty()) {
                    for (int i = 1; i < this->PCounter; i += this->sboxExtWeighted.size()) {
                        int k = 0;
                        for (auto weight: this->sboxExtWeighted) {
                            if (weight != 1)
                                target << weight << " P" << i + k;
                            else
                                target << "P" << i + k;
                            if (i + k != this->PCounter - 1) target << " + ";
                            k++;
                        }
                    }
                } else {
                    for (int i = 1; i < this->PCounter; i ++) {
                        target << "P" << i;
                        if (i != this->PCounter - 1) target << " + ";
                    }
                }
            }
            target << "\n" << "Subject To\n";

            // speedup constraints
            // 每轮的input difference 累加和都要大于0
            if (this->speedup1) {
                for (int j = 0; j < this->differentialCharacteristic.size() - 1; ++j) {
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        if (k != this->differentialCharacteristic[j].size() - 1)
                            target << "x" << std::to_string(this->differentialCharacteristic[j][k]) << " + ";
                        else
                            target << "x" << std::to_string(this->differentialCharacteristic[j][k]);
                    }
                    if (this->differentialCharacteristic[j].size() > 0) target << " >= 1\n";
                }
            } else {
                // related key的初始约束是所有key的差分的累加和大于等于1
                for (int i = 0; i < this->keySize; ++i) {
                    if (i != this->keySize - 1)
                        target << "x" << std::to_string(i + 1) << " + ";
                    else
                        target << "x" << std::to_string(i + 1);
                }
                target << " >= 1\n";
            }

            if (this->speedup2) {
                if (this->mode == "AS") {
                    for (int i = 0; i < this->finalResults.size(); ++i) {
                        for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                            for (int k = 0; k < this->allRoundACounters[j].size(); ++k) {
                                target << "A" << this->allRoundACounters[j][k];
                                if (k != this->allRoundACounters[j].size() - 1) target << " + ";
                            }
                            if (j != this->currentRound - 1) target << " + ";
                        }
                        target << " - " << this->finalResults[i] << " >= 0\n";
                    }

                    for (int i = 0; i < this->finalResults.size(); ++i) {
                        for (int j = 0; j < i + 1; ++j) {
                            for (int k = 0; k < this->allRoundACounters[j].size(); ++k) {
                                target << "A" << this->allRoundACounters[j][k];
                                if (k != this->allRoundACounters[j].size() - 1) target << " + ";
                            }
                            if (j != i) target << " + ";
                        }
                        target << " - " << this->finalResults[i] << " >= 0\n";
                    }
                } else if (this->mode == "DC") {
                    if (!this->sboxExtWeighted.empty()) {
                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); k += this->sboxExtWeighted.size()) {
                                    int p = 0;
                                    for (auto weight: this->sboxExtWeighted) {
                                        if (weight != 1) {
                                            target << std::to_string(weight) << " P" << this->allRoundPCounters[j][k + p];
                                        } else {
                                            target << "P" << this->allRoundPCounters[j][k + p];
                                        }
                                        if (k + p != this->allRoundPCounters[j].size() - 1) target << " + ";
                                        p++;
                                    }
                                }
                                if (j != this->currentRound - 1) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }

                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = 0; j < i + 1; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); k += this->sboxExtWeighted.size()) {
                                    int p = 0;
                                    for (auto weight: this->sboxExtWeighted) {
                                        if (weight != 1) {
                                            target << std::to_string(weight) << " P" << this->allRoundPCounters[j][k + p];
                                        } else {
                                            target << "P" << this->allRoundPCounters[j][k + p];
                                        }
                                        if (k + p != this->allRoundPCounters[j].size() - 1) target << " + ";
                                        p++;
                                    }
                                }
                                if (j != i) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }
                    } else {

                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); ++k) {
                                    target << "P" << this->allRoundPCounters[j][k];
                                    if (k != this->allRoundPCounters[j].size() - 1) target << " + ";
                                }
                                if (j != this->currentRound - 1) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }

                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = 0; j < i + 1; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); ++k) {
                                    target << "P" << this->allRoundPCounters[j][k];
                                    if (k != this->allRoundPCounters[j].size() - 1) target << " + ";
                                }
                                if (j != i) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }


                    }
                }
            }

            target << model;
            // Here we need to add a constraint, since there may be modeling of zeroing after shift in the subsequent matrix,
            // and all the zeroing bits are represented by x_0, hence we initial the value of x_0 to 0 here
            target << "x0 = 0\n";

        }
        target.close();
    }
    std::ofstream binary(this->modelPath, std::ios::app);
    if (!binary) {
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        binary << "Binary\n";
        // ILP or MILP ?
        if (this->ILPFlag) {
            for (int i = 1; i < this->xCounter; ++i)
                binary << "x" << i << "\n";
        } else {
            for (int i = 1; i < this->keySize + 1; ++i) {
                binary << "x" << i << "\n";
            }
        }
        for (int i = 1; i < this->dCounter; ++i)
            binary << "d" << i << "\n";
        if (this->mode == "AS") {
            for (int i = 1; i < this->ACounter; ++i)
                binary << "A" << i << "\n";
        } else if (this->mode == "DC") {
            for (int i = 1; i < this->PCounter; ++i)
                binary << "P" << i << "\n";
        }
        for (int i = 1; i < this->fCounter; ++i)
            binary << "f" << i << "\n";
        // integer variables
        binary << "General\n";
        for (int i = 0; i < this->yCounter; ++i)
            binary << "y" << i << "\n";
        binary << "End";
        binary.close();
    }
}


void DiffRBMILP::evaluation() {

    if (this->mode == "AS") {
        // 这里我还需要想一下，targNdiff究竟应该是选择上界ceil还是下界floor
        // 现在分析来看，暂时选择上界ceil
        this->targetNdiffOrPr = int(ceil(this->currentRound * ((this->keySize / this->sboxMDP) / this->totalRoundNum))) - 1;
    } else if (this->mode == "DC") {
        this->targetNdiffOrPr = int(ceil(this->currentRound * (this->keySize / this->totalRoundNum))) - 1;

        if (this->cipherName == "GIFT_128" or this->cipherName == "GIFT_64")
            this->targetNdiffOrPr = int(this->currentRound * this->keySize / this->totalRoundNum * 200);
    }


    // debug
    std::cout << "currMaxSearchRound : " << this->currentRound << std::endl;
    std::cout << "sboxNdiffLowerBoundAvarange : " << ((this->keySize / this->sboxMDP) / this->totalRoundNum) << std::endl;
    std::cout << "prUpperBoundAverage : " << (this->keySize / this->totalRoundNum) << std::endl;
    std::cout << "targetNdiff : " << targetNdiffOrPr << std::endl;

    programGenModel();

    std::ifstream file;
    file.open(this->modelPath);
    std::string model,line;
    while (getline(file, line)) {model += line + "\n";}
    file.close();

    if (model != "") {
        std::ofstream target(this->modelPath, std::ios::trunc);
        if (!target) {
            std::cout << "Wrong file path ! " << std::endl;
        } else {
            target << "Subject To\n";
            if (this->mode == "AS") {
                for (int i = 1; i < this->ACounter; ++i) {
                    target << "A" << i;
                    if (i != this->ACounter - 1) target << " + ";
                }
            } else if (this->mode == "DC") {
                if (!this->sboxExtWeighted.empty()) {
                    for (int i = 1; i < this->PCounter; i += this->sboxExtWeighted.size()) {
                        int k = 0;
                        for (auto weight: this->sboxExtWeighted) {
                            if (weight != 1)
                                target << weight << " P" << i + k;
                            else
                                target << "P" << i + k;
                            if (i + k != this->PCounter - 1) target << " + ";
                            k++;
                        }
                    }
                } else {
                    for (int i = 1; i < this->PCounter; i ++) {
                        target << "P" << i;
                        if (i != this->PCounter - 1) target << " + ";
                    }
                }
            }
            target << " <= " << this->targetNdiffOrPr << "\n";

            // speedup constraints
            // 每轮的input difference 累加和都要大于0
            if (this->speedup1) {
                for (int j = 0; j < this->differentialCharacteristic.size() - 1; ++j) {
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        if (k != this->differentialCharacteristic[j].size() - 1)
                            target << "x" << std::to_string(this->differentialCharacteristic[j][k]) << " + ";
                        else
                            target << "x" << std::to_string(this->differentialCharacteristic[j][k]);
                    }
                    if (this->differentialCharacteristic[j].size() > 0) target << " >= 1\n";
                }
            } else {
                for (int i = 0; i < this->blockSize; ++i) {
                    if (i != this->blockSize - 1)
                        target << "x" << std::to_string(i + 1) << " + ";
                    else
                        target << "x" << std::to_string(i + 1);
                }
                target << " >= 1\n";
            }

            if (this->speedup2) {
                if (this->mode == "AS") {
                    for (int i = 0; i < this->finalResults.size(); ++i) {
                        for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                            for (int k = 0; k < this->allRoundACounters[j].size(); ++k) {
                                target << "A" << this->allRoundACounters[j][k];
                                if (k != this->allRoundACounters[j].size() - 1) target << " + ";
                            }
                            if (j != this->currentRound - 1) target << " + ";
                        }
                        target << " - " << this->finalResults[i] << " >= 0\n";
                    }

                    for (int i = 0; i < this->finalResults.size(); ++i) {
                        for (int j = 0; j < i + 1; ++j) {
                            for (int k = 0; k < this->allRoundACounters[j].size(); ++k) {
                                target << "A" << this->allRoundACounters[j][k];
                                if (k != this->allRoundACounters[j].size() - 1) target << " + ";
                            }
                            if (j != i) target << " + ";
                        }
                        target << " - " << this->finalResults[i] << " >= 0\n";
                    }
                } else if (this->mode == "DC") {
                    if (!this->sboxExtWeighted.empty()) {
                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); k += this->sboxExtWeighted.size()) {
                                    int p = 0;
                                    for (auto weight: this->sboxExtWeighted) {
                                        if (weight != 1) {
                                            target << std::to_string(weight) << " P" << this->allRoundPCounters[j][k + p];
                                        } else {
                                            target << "P" << this->allRoundPCounters[j][k + p];
                                        }
                                        if (k + p != this->allRoundPCounters[j].size() - 1) target << " + ";
                                        p++;
                                    }
                                }
                                if (j != this->currentRound - 1) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }

                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = 0; j < i + 1; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); k += this->sboxExtWeighted.size()) {
                                    int p = 0;
                                    for (auto weight: this->sboxExtWeighted) {
                                        if (weight != 1) {
                                            target << std::to_string(weight) << " P" << this->allRoundPCounters[j][k + p];
                                        } else {
                                            target << "P" << this->allRoundPCounters[j][k + p];
                                        }
                                        if (k + p != this->allRoundPCounters[j].size() - 1) target << " + ";
                                        p++;
                                    }
                                }
                                if (j != i) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }
                    } else {


                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = this->currentRound - i - 1; j < this->currentRound; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); ++k) {
                                    target << "P" << this->allRoundPCounters[j][k];
                                    if (k != this->allRoundPCounters[j].size() - 1) target << " + ";
                                }
                                if (j != this->currentRound - 1) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }

                        for (int i = 0; i < this->finalResults.size(); ++i) {
                            for (int j = 0; j < i + 1; ++j) {
                                for (int k = 0; k < this->allRoundPCounters[j].size(); ++k) {
                                    target << "P" << this->allRoundPCounters[j][k];
                                    if (k != this->allRoundPCounters[j].size() - 1) target << " + ";
                                }
                                if (j != i) target << " + ";
                            }
                            target << " - " << this->finalResults[i] << " >= 0\n";
                        }


                    }

                }
            }

            target << model;
            target << "x0 = 0\n";

        }
        target.close();
    }
    std::ofstream binary(this->modelPath, std::ios::app);
    if (!binary) {
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        binary << "Binary\n";
        // ILP or MILP ?
        if (this->ILPFlag) {
            for (int i = 1; i < this->xCounter; ++i)
                binary << "x" << i << "\n";
        } else {
            for (int i = 1; i < this->keySize + 1; ++i) {
                binary << "x" << i << "\n";
            }
        }
        for (int i = 1; i < this->dCounter; ++i)
            binary << "d" << i << "\n";
        if (this->mode == "AS") {
            for (int i = 1; i < this->ACounter; ++i)
                binary << "A" << i << "\n";
        } else if (this->mode == "DC") {
            for (int i = 1; i < this->PCounter; ++i)
                binary << "P" << i << "\n";
        }
        for (int i = 1; i < this->fCounter; ++i)
            binary << "f" << i << "\n";
        // integer variables
        binary << "General\n";
        for (int i = 0; i < this->yCounter; ++i)
            binary << "y" << i << "\n";
        binary << "End";
        binary.close();
    }
}


void DiffRBMILP::solver() {
    clock_t startTime, endTime;
    startTime = clock();
    time_t star_time = 0, end_time;
    star_time = time(NULL);
    GRBEnv env = GRBEnv(true);

    // setup threads num
    env.set(GRB_IntParam_Threads, this->gurobiThreads);
    env.set(GRB_IntParam_MIPFocus, 3);
    env.start();
    GRBModel model = GRBModel(env, modelPath);
    model.set(GRB_IntParam_MIPFocus, 2);
    // timer
    model.set(GRB_DoubleParam_TimeLimit, this->gurobiTimer);
    model.optimize();
    endTime = clock();
    end_time = time(NULL);

    int optimstatus = model.get(GRB_IntAttr_Status);
    int result = 0;
    double clockTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    double timeTime = difftime( end_time, star_time);

    if (this->target == "cryptanalysis") {
        if (optimstatus == 2) {
            result = model.get(GRB_DoubleAttr_ObjVal);

            std::ofstream f(this->resultsPath, std::ios::trunc);
            f << "solving the MILP " << this->modelPath << "\n";
            f << "obj is : " << result << "\n";
            f << "clock time : " << clockTime << "s\n";
            f << "time time : " << timeTime << "s\n";

            // Obtaining the specific values of variables
            // all variable and results mapping
            map<std::string, int> dcVarMaps;
            GRBVar *a = model.getVars();
            int varNum = model.get(GRB_IntAttr_NumVars);
            for (int i = 0; i < varNum; ++i) {
                f << a[i].get(GRB_StringAttr_VarName) << " = " << a[i].get(GRB_DoubleAttr_X) << "\n";
                dcVarMaps[a[i].get(GRB_StringAttr_VarName)] = int(a[i].get(GRB_DoubleAttr_X));
            }
            f.close();

            // save differential characteristic
            std::ofstream dcs(this->diffCharacteristicPath, std::ios::trunc);
            if (!dcs){
                std::cout << "Wrong file path ! " << std::endl;
            } else {
                for (int j = 0; j < this->differentialCharacteristic.size(); ++j) {
                    if (j%2 == 0)
                        dcs << j/2 << "-th input difference\n";
                    else
                        dcs << (j-1)/2 << "-th output difference\n";
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        dcs << dcVarMaps["x" + std::to_string(this->differentialCharacteristic[j][k])];
                        if ((k+1)%8 == 0)
                            dcs << " ";
                        if (k == this->differentialCharacteristic[j].size() - 1)
                            dcs << "\n";
                    }
                }
            }
            dcs.close();

            std::cout << "***********************************" << std::endl;
            std::cout << "      Obj is  : " << result << std::endl;
            std::cout << "***********************************" << std::endl;
            this->finalResults.push_back(result);
            this->finalClockTime.push_back(clockTime);
            this->finalTimeTime.push_back(timeTime);
        } else if (optimstatus == 3) {
            std::cout << "************************* THE CONSTRUCTED MODEL ARE INFEASIBLE *************************" << std::endl;
        }
    } else if (this->target == "evaluation") {
        if (optimstatus == 2) {
            std::ofstream f(this->resultsPath, std::ios::trunc);
            f << "solving the MILP " << this->modelPath << "\n";
            f << "obj is : " << this->targetNdiffOrPr << "\n";
            f << "clock time : " << clockTime << "s\n";
            f << "time time : " << timeTime << "s\n";

            // Obtaining the specific values of variables
            // all variable and results mapping
            map<std::string, int> dcVarMaps;
            GRBVar *a = model.getVars();
            int varNum = model.get(GRB_IntAttr_NumVars);
            for (int i = 0; i < varNum; ++i) {
                f << a[i].get(GRB_StringAttr_VarName) << " = " << a[i].get(GRB_DoubleAttr_X) << "\n";
                dcVarMaps[a[i].get(GRB_StringAttr_VarName)] = int(a[i].get(GRB_DoubleAttr_X));
            }
            f.close();

            // save differential characteristic
            std::ofstream dcs(this->diffCharacteristicPath, std::ios::trunc);
            if (!dcs){
                std::cout << "Wrong file path ! " << std::endl;
            } else {
                for (int j = 0; j < this->differentialCharacteristic.size(); ++j) {
                    if (j%2 == 0)
                        dcs << j/2 << "-th input difference\n";
                    else
                        dcs << (j-1)/2 << "-th output difference\n";
                    for (int k = 0; k < this->differentialCharacteristic[j].size(); ++k) {
                        dcs << dcVarMaps["x" + std::to_string(this->differentialCharacteristic[j][k])];
                        if ((k+1)%8 == 0)
                            dcs << " ";
                        if (k == this->differentialCharacteristic[j].size() - 1)
                            dcs << "\n";
                    }
                }
            }
            dcs.close();

            std::cout << "***********************************" << std::endl;
            std::cout << "      number of active sboxes  : " << this->targetNdiffOrPr << std::endl;
            std::cout << "      clockTime is  : " << clockTime << std::endl;
            std::cout << "      timeTime is  : " << timeTime << std::endl;
            std::cout << "***********************************" << std::endl;
            this->finalResults.push_back(this->targetNdiffOrPr);
            this->finalClockTime.push_back(clockTime);
            this->finalTimeTime.push_back(timeTime);
        }
        else if (optimstatus == 3) {
            // 如果模型不可满足，说明无法找到小于等于数量为targetNdiff的differential characteristic，
            // 即说明任意的differential characteristic的active sbox数量至少大于targetNdiff，
            // 或者无法找到小于等于某个数（对应于概率的上界），即最大差分概率的上界在某个值时，最大差分概率依然小于等于2^{-k},
            // 亦即block cipher是足够安全的
            this->securityFlag = true;
            this->securityRoundNumBound = this->currentRound;
            std::cout << "\n\n\n****************************************************************" << std::endl;
            std::cout << "the block cipher is security enough when the round number is : "
                      << this->currentRound << std::endl;
            std::cout << "****************************************************************\n\n\n" << std::endl;

            std::cout << "***********************************" << std::endl;
            std::cout << "      clockTime is  : " << clockTime << std::endl;
            std::cout << "      timeTime is  : " << timeTime << std::endl;
            std::cout << "***********************************" << std::endl;
            this->finalClockTime.push_back(clockTime);
            this->finalTimeTime.push_back(timeTime);

            std::ofstream f(this->resultsPath, std::ios::trunc);
            f << "clock time : " << clockTime << "s\n";
            f << "time time : " << timeTime << "s\n";
            f.close();

            // 如果在模型不可解时，需要继续求解，并且找出对应轮数下的最少活跃active数量，就是用下面注释的代码
            /*if (this->cipherName == "GIFT_128" or this->cipherName == "GIFT_64")
                this->targetNdiffOrPr += 200;
            else
                this->targetNdiffOrPr++;
            std::ofstream target(this->modelPath, std::ios::trunc);
            if (!target) {
                std::cout << "Wrong file path ! " << std::endl;
            } else {
                target << this->satTargetObj.substr(0, this->satTargetObj.size() - 1);
                target << " = " << targetNdiffOrPr << "\n";
                target << this->satModel;
                target.close();
            }
            solver();*/
        }
        else
            std::cout << "optimstatus : " << optimstatus << std::endl;
    }
}


void DiffRBMILP::programGenModel() {
    int roundCounter = this->currentRound;

    for (const auto& proc : this->procedureHs) {
        if (proc->getName() == "main") {

            /*
             * 之前的思考是，每一个round function后都会有一个key schedule调用，但是实际上，可能存在在
             * 所有的round function之前统一完成key schedule的调用，当然也可能有其他调用顺序，但是核心
             * 在于，对于DSL而言，不应在此处对round function和key schedule的调用顺序有任何限制，因为
             * 程序语言的编程应该是在符合语法的条件下任意进行而非拘泥于某些固定格式，那么在这种情形下我们如
             * 何设计统一的分析方式呢？
             *
             * 思路分析：
             * 每一个round function都应该有单独的建模，而round function的多次调用之间，应该需要不断地
             * 更新其中变量对应的xCounter，以保证在不同轮数时的约束构建不会相互重复。
             * 故类似的，每一个key schedule应该也都有着对应的建模，并且保证key schedule的多次调用之间不
             * 会有相互影响，也即不同轮数的相同变量之间要保证xCounter的对应更新。
             * 那么还有一个需要处理的问题就是，在main函数中，round function和key schedule调用之间的协
             * 调问题，一个直观的想法是，this->tanNameMxIndex中始终维护者main函数和round function中
             * 所有变量对应着的xCounter，在round function被调用被分析时，可以直接通过其中的映射来更新形
             * 式参数和实际参数的映射。然后再用另外一个this->kschdNameMxIndex用于维护key schedule建模
             * 时的映射，并且在main函数的调用前后来不断地更新this->tanNameMxIndex中实际的key对应的xCounter，
             * 以保证round function中的key可以对应建模。
             *
             * 那么如何确定每一轮的建模结束了呢？
             * 之前的思路想着，每一个round function后都会跟随着一个key schedule，那么当两种函数都被调用
             * 一次时，意味着一轮结束。但是这种设计无法解决编程任意实现的问题，所以正确的做法应该是，以round
             * function作为每一轮的划分，key schedule直接独立进行即可。
             *
             * */

            // related key 模式下，分析之前，给定素有的key都有一个对应的xCounter，使得在第一轮round function调用前，
            // 即使没有调用key schedule，也可以有对应key的xCounter
            // 并且需要根据input来确定keySize
            for (const auto & i : proc->getParameters().at(0)) {
                this->tanNameMxIndex["main_" + i->getNodeName()] = this->xCounter;
                this->xCounter++;
            }
            this->keySize = proc->getParameters().at(0).size();

            std::string roundFuncId, kschdFuncId;
            bool newRoundFlag = false, roundFuncCallFlag = false;
            int tempBlockSizeCounter = 0;
            // round function 中参数key的实际参数
            std::vector<ThreeAddressNodePtr> argumentKey;

            for (int idx = 0; idx < proc->getBlock().size(); ++idx) {
                ThreeAddressNodePtr ele = proc->getBlock().at(idx);
                // When the value of the first parameter "round" is queried,
                // it means that a round function call has already started
                if (ele->getLhs()->getNodeType() == UINT and ele->getNodeName() == "push") {
                    newRoundFlag = true;
                } else if (newRoundFlag and roundCounter > 0) {
                    if (ele->getNodeName() == "plaintext_push")
                        tempBlockSizeCounter++;
                    else if (ele->getNodeName() == "key_push") {
                        argumentKey.push_back(ele->getLhs());
                    }
                    // start function call
                    else if (ele->getOp() == ASTNode::CALL) {
                        this->blockSize = tempBlockSizeCounter;
                        tempBlockSizeCounter = 0;
                        roundFuncId = ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@"));
                        for (const auto& tproc : this->procedureHs) {
                            if (tproc->getName() == roundFuncId) {
                                int rtnSize = tproc->getReturns().size();
                                // 这里用于提取每轮的 ACounter，用于记录每轮的 target function 中的 ACounter
                                roundFunctionGenModel(tproc, argumentKey);
                                roundCounter--;
                                argumentKey.clear();
                                if (this->mode == "AS") {
                                    int startACounter = this->lastRoundACounter;
                                    int endACounter = this->ACounter;
                                    this->lastRoundACounter = this->ACounter;
                                    std::vector<int> roundACounter;
                                    for (int i = startACounter; i < endACounter; ++i) {
                                        roundACounter.push_back(i);
                                    }
                                    this->allRoundACounters.push_back(roundACounter);
                                } else if (this->mode == "DC") {
                                    int startPCounter = this->lastRoundPCounter;
                                    int endPCounter = this->PCounter;
                                    this->lastRoundPCounter = this->PCounter;
                                    std::vector<int> roundPCounter;
                                    for (int i = startPCounter; i < endPCounter; ++i) {
                                        roundPCounter.push_back(i);
                                    }
                                    this->allRoundPCounters.push_back(roundPCounter);
                                }
                                roundFuncCallFlag = true;
                                // 每调用一次round function，直接进行下一个非函数调用的操作的建模，
                                // 因为每个round function返回值是一个数组，已经被拆分成多个三地址实例，
                                // 而这么多三地址实例的建模应该是同一个round function，因此只需要一次即可
                                ele = proc->getBlock().at(idx + rtnSize);
                                break;
                            }
                        }
                        newRoundFlag = false;
                    } else
                        assert(false);

                    // 如果在本次round function执行完以后，roundCounter已经为0，就说明建模已经结束了
                    if (roundCounter <= 0) {
                        break;
                    }
                }
                // main 函数中直接调用key schedule
                // 遇到kschd_push，就寻找kscd的函数调用，然后直接处理函数调用
                else if (ele->getNodeName() == "kschd_push") {
                    std::vector<ThreeAddressNodePtr> kschdInput, kschdOutput;
                    while (true) {
                        ele = proc->getBlock().at(idx);
                        if (ele->getOp() == ASTNode::CALL) {
                            kschdOutput.push_back(ele);
                        } else if (ele->getNodeName() == "kschd_push") {
                            kschdInput.push_back(ele->getLhs());
                        } else {
                            idx--;
                            ele = proc->getBlock().at(idx);
                            kschdFuncId = ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find(
                                    "@"));
                            break;
                        }
                        idx++;
                    }
                    this->keySize = kschdInput.size();
                    // 找到key schedule建模函数的input和output，就开始建模
                    for (const auto &tproc: this->procedureHs) {
                        if (tproc->getName() == kschdFuncId) {
                            keyScheduleGenModel(tproc, kschdInput, kschdOutput);
                            break;
                        }
                    }
                }

                // If the previous model is empty, it means that the previous round will not generate the model,
                // and the related member variables need to be cleared, making sure that it does not affect the
                // subsequent model generation.
                if (roundFuncCallFlag) {
                    std::ifstream file;
                    file.open(this->modelPath);
                    std::string model, line;
                    while (getline(file, line)) {model += line + "\n";}
                    file.close();
                    if (model == "") {
                        // initial for next round;
                        this->tanNameMxIndex.clear();
                        this->rtnMxIndex.clear();
                        this->rtnIdxSave.clear();
                        this->kschdNameMxIndex.clear();
                        this->kschdRtnIdxSave.clear();
                        this->xCounter = 1;
                        this->dCounter = 1;
                        this->ACounter = 1;
                        this->PCounter = 1;
                        this->fCounter = 1;
                        this->yCounter = 1;
                        this->lastRoundACounter = 1;
                        this->lastRoundPCounter = 1;
                        this->allRoundACounters.clear();
                        this->allRoundPCounters.clear();
                        roundFuncCallFlag = false;
                        this->differentialCharacteristic.clear();
                    }
                }

            }
            break;
        }
    }
}


// 对于related key的建模，所有的key我们都不能忽略，因此我们需要也传入key的实参，以用于更新key的xCoutner的映射
void DiffRBMILP::roundFunctionGenModel(const ProcedureHPtr &procedureH, std::vector<ThreeAddressNodePtr> inputKey) {
    std::string keyId = procedureH->getParameters().at(1).at(0)->getNodeName().substr(0, procedureH->getParameters().at(1).at(0)->getNodeName().find("0"));

    // related key的建模也需要映射虚参数key的xCounter
    for (int i = 0; i < procedureH->getParameters().at(1).size(); ++i) {
        this->tanNameMxIndex[procedureH->getParameters().at(1).at(i)->getNodeName()] = this->tanNameMxIndex["main_" + inputKey[i]->getNodeName()];
    }


    // At the beginning of each round, we need to store the result of the previous round of "return" to xIndex,
    // and the three address instances corresponding to each index are the input plaintext of the round function
    if (!this->rtnIdxSave.empty()) {
        for (int i = 0; i < this->rtnIdxSave.size(); ++i)
            this->tanNameMxIndex[procedureH->getParameters().at(2).at(i)->getNodeName()] = this->rtnIdxSave[i];
        // 当从rtnIdxSave中获取differential时，只能从至少第二轮开始，
        // 但是因为本函数末尾，我们同样会pushback output differential characteristic，
        // 所以我们这里就不在pushback 第二轮开始的 differential characteristic
        // save input difference
        /*std::vector<int> inputC;
        for (int & i : this->rtnIdxSave) {
            inputC.push_back(i);
        }
        this->differentialCharacteristic.push_back(inputC);*/
        this->rtnIdxSave.clear();
        this->rtnMxIndex.clear();
    } else {
        // 在related key模式时，roundFunction的xCounter不是从0开始的，因为还有keySchedule的建模，
        // 所以我们需要找到roundFunction的xCounter的起始位置
        int inputXCounterStart;
        for (int i = 0; i < procedureH->getParameters().at(2).size(); ++i) {
            if (i == 0)
                inputXCounterStart = this->xCounter;
            this->tanNameMxIndex[procedureH->getParameters().at(2)[i]->getNodeName()] = this->xCounter;
            this->xCounter++;
        }
        /*for (const auto & i : procedureH->getParameters().at(2)) {
            this->tanNameMxIndex[i->getNodeName()] = this->xCounter;
            this->xCounter++;
        }*/
        // initial input difference
        std::vector<int> inputC;
        for (int i = inputXCounterStart; i < this->xCounter; ++i) {
            inputC.push_back(i);
        }
        this->differentialCharacteristic.push_back(inputC);
    }


    bool functionCallFlag;
    for (int i = 0; i < procedureH->getBlock().size(); ++i) {
        functionCallFlag = false;
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);

        if (ele->getOp() == ASTNode::XOR) {
            // XOR with constants does not affect differential propagation
            if (this->isConstant(ele->getLhs()) or this->isConstant(ele->getRhs())) {}
            else {
                functionCallFlag = true;
                XORGenModel(ele->getLhs(), ele->getRhs(), ele, false, false);
            }
        } else if (ele->getOp() == ASTNode::AND or ele->getOp() == ASTNode::OR) {
            functionCallFlag = true;
            ANDandORGenModel(ele->getLhs(), ele->getRhs(), ele, false, false);
        } else if (ele->getOp() == ASTNode::BOXOP) {
            functionCallFlag = true;
            if (ele->getLhs()->getNodeName().substr(0, 4) == "sbox") {
                // 不管是数组类型的sbox op或者uints类型的sbox op，都可以统一处理，因为我们在三地址转换的时候，
                // 所有的赋值语句都会生成对应的三地址，所以此时所有的boxop都会被统一直接处理，不会存在再symbolIndex类型中再进行
                // boxop的模型生成了
                SboxGenModel(ele->getLhs(), ele->getRhs(), ele, false);
            } else if (ele->getLhs()->getNodeName().substr(0, 4) == "pbox") {
                PboxGenModel(ele->getLhs(), ele->getRhs(), ele, false);
            }
        }
        else if (ele->getOp() == ASTNode::ADD) {
            functionCallFlag = true;
            std::string outputName = ele->getNodeName().substr(0, ele->getNodeName().find_last_of("_"));
            std::vector<ThreeAddressNodePtr> input1, input2, output;
            while (true) {
                input1.push_back(ele->getLhs());
                input2.push_back(ele->getRhs());
                output.push_back(ele);
                i++;
                if (i == procedureH->getBlock().size()) {
                    i--;
                    break;
                }
                ele = procedureH->getBlock().at(i);
                if (ele != nullptr and ele->getOp() == ASTNode::ADD and ele->getNodeName().substr(0, outputName.size()) == outputName)
                    continue;
                else {
                    i--;
                    break;
                }
            }
            ADDandMINUSGenModel(input1, input2, output, 1, false, false);
        }
        else if (ele->getOp() == ASTNode::MINUS) {
            if (ele->getLhs()->getNodeType() == NodeType::UINT or ele->getRhs()->getNodeType() == NodeType::UINT) {
                continue;
            }

            // 目前发现的情况是有关轮数r，即第一个参数的减法。
            // 此时，因为轮数r是可以evaluate的，因为在传参时，r是确定的，所以需要在这里evaluate结果
            // 暂时未确定，后续需要check所有的benchmark情况

            if (isConstant(ele->getLhs()) or isConstant(ele->getRhs())) {

            } else {
                functionCallFlag = true;
                std::string outputName = ele->getNodeName().substr(0, ele->getNodeName().find_last_of("_"));
                std::vector<ThreeAddressNodePtr> input1, input2, output;
                while (true) {
                    input1.push_back(ele->getLhs());
                    input2.push_back(ele->getRhs());
                    output.push_back(ele);
                    i++;
                    if (i == procedureH->getBlock().size()) {
                        i--;
                        break;
                    }
                    ele = procedureH->getBlock().at(i);
                    std::string  aaa = ele->getNodeName().substr(0, outputName.size());

                    if (ele != nullptr and ele->getOp() == ASTNode::ADD and ele->getNodeName().substr(0, outputName.size()) == outputName)
                        continue;
                    else {
                        i--;
                        break;
                    }
                }
                ADDandMINUSGenModel(input1, input2, output, 2, false, false);
            }
        } else if (ele->getOp() == ASTNode::PUSH) {
            assert(false);
            functionCallFlag = true;
            std::vector<ThreeAddressNodePtr> input, output;
            while (ele->getNodeName() == "sbox_push") {
                input.push_back(ele->getLhs());
                i++;
                ele = procedureH->getBlock().at(i);
            }
            std::string sboxFuncId = ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@"));
            while (ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@")) == sboxFuncId) {
                output.push_back(ele);
                i++;
                ele = procedureH->getBlock().at(i);
            }
            i--;
            ele = procedureH->getBlock().at(i);
            for (const auto& tproc : this->procedureHs) {
                if (tproc->getName() == sboxFuncId) {
                    // 这里用于提取每轮的 ACounter，用于记录每轮的 target function 中的 ACounter
                    sboxFunctionGenModel(tproc, input, output);
                    break;
                }
            }
        } else if (ele->getOp() == ASTNode::SYMBOLINDEX) {
            // ffm 乘法在转换成三地址以后会被拆分成单个元素，因此需要先判断是否是symbolIndex然后再判断左孩子结点的operation
            ThreeAddressNodePtr left = ele->getLhs();
            if (left->getOp() == ASTNode::FFTIMES) {
                if (left->getLhs()->getNodeName().substr(0, 5) == "pboxm") {
                    functionCallFlag = true;
                    std::vector<ThreeAddressNodePtr> output;
                    int pboxSize = pboxM[left->getLhs()->getNodeName()].size();
                    int outputNum = (int)sqrt(pboxSize);
                    while (outputNum > 0) {
                        output.push_back(ele);
                        i++;
                        if (i == procedureH->getBlock().size()) {
                            i--;
                            break;
                        } else {
                            ele = procedureH->getBlock().at(i);
                            left = ele->getLhs();
                            if (left != nullptr and left->getOp() == ASTNode::FFTIMES and
                                left->getLhs()->getNodeName().substr(0, 5) == "pboxm") {
                                outputNum--;
                                continue;
                            } else {
                                i--;
                                left = procedureH->getBlock().at(i)->getLhs();
                                break;
                            }
                        }
                    }
                    if (outputNum == 0) i--;
                    left = procedureH->getBlock().at(i)->getLhs();
                    MatrixVectorGenModel(left->getLhs(), left->getRhs(), output);
                } else
                    assert(false);
            } else if (left->getOp() == ASTNode::BOXOP) {
                // box op 已经被处理
            } else
                assert(false);
        } else if (ele->getOp() == ASTNode::TOUINT) {
            // touint需要根据目前的ele名字拆分成为s个1bit的三地址实例，并根据touint的链接对象依次分配对应的xCounter
            if (ele->getLhs()->getOp() == ASTNode::BOXINDEX) {
                ThreeAddressNodePtr left = ele->getLhs();
                std::vector<int> leftIdx = extIdxFromTOUINTorBOXINDEX(left, false, false);
                int size = transNodeTypeSize(ele->getNodeType());
                for (int j = 0; j < size; ++j) {
                    this->tanNameMxIndex[ele->getNodeName() + "_$B$_" + std::to_string(j)] = leftIdx[j];
                }
            }
        } else
            assert(false);

        // If no function call is performed, it means that this node does not participate in any constraint generation operation.
        // At this time, if the left and right child nodes have been stored in xIndex, we need to overwrite their original nodes,
        // because subsequent operations need to be carried out on their basis.
        // Here we still use name of node to judge, because different pointers may point to the same memory address.
        if (!functionCallFlag) {
            // 若左孩子结点为array或者touint(即被操作对象是array或者是touint),注意，此时无论是array还是touint，其三地址实例的类型均为array,
            // 且操作符为 symbolIndex，即取array中的某个元素，元素索引为右孩子结点对应的值
            if (ele->getLhs()->getNodeType() == NodeType::ARRAY and ele->getOp() == ASTNode::SYMBOLINDEX) {
                std::string finderName = ele->getLhs()->getNodeName() + "_$B$_" + ele->getRhs()->getNodeName();
                for (const auto &pair: this->tanNameMxIndex) {
                    if (finderName == pair.first)
                        this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                }
            }
                // 若孩子节点不是array，并且没有进行模型生成的函数调用，那么就只是对某个uint1类型的变量进行操作，只需要沿用孩子节点对应的xCounter即可
            else {
                for (const auto &pair: this->tanNameMxIndex) {
                    if (ele->getLhs() != nullptr)
                        if (ele->getLhs()->getNodeName() == pair.first)
                            this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                    if (ele->getRhs() != nullptr)
                        if (ele->getRhs()->getNodeName() == pair.first)
                            this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                }
            }
        } else
            functionCallFlag = false;
    }

    for (const auto& rtn : procedureH->getReturns()) {
        for (const auto &pair: this->tanNameMxIndex) {
            if (rtn->getNodeName() == pair.first) {
                this->rtnMxIndex[pair.first] = pair.second;
                this->rtnIdxSave.push_back(pair.second);
            }
        }
    }

    // output difference
    std::vector<int> outputC;
    for (int & i : this->rtnIdxSave) {
        outputC.push_back(i);
    }
    this->differentialCharacteristic.push_back(outputC);
}


void DiffRBMILP::keyScheduleGenModel(const ProcedureHPtr &procedureH, std::vector<ThreeAddressNodePtr> input, std::vector<ThreeAddressNodePtr> output) {
    this->kschdNameMxIndex.clear();
    std::vector<int> inputIdx;

    // debug
    // std::cout << "\n ------------------------ key schedule input index ------------------------ \n" << std::endl;

    int idx = 0;
    // 不同key schedule之间的建模是独立的，因此只需要根据tanNameMxIndex的实参的map来更新在kschdNameMxIndex中形参的map即可
    for (const auto & i : procedureH->getParameters().at(0)) {
        if (this->tanNameMxIndex.count("main_" + input[idx]->getNodeName()) != 0) {
            this->kschdNameMxIndex[i->getNodeName()] = this->tanNameMxIndex["main_" + input[idx]->getNodeName()];

            // std::cout << i->getNodeName() << " : " << "main_" + input[idx]->getNodeName() << " : " << this->tanNameMxIndex[input[idx]->getNodeName()] << std::endl;

            inputIdx.push_back(this->tanNameMxIndex["main_" + input[idx]->getNodeName()]);
            idx++;
        } else
            assert(false);
    }

    bool functionCallFlag;
    for (int i = 0; i < procedureH->getBlock().size(); ++i) {
        functionCallFlag = false;
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);
        if (ele->getOp() == ASTNode::XOR) {
            if (this->isConstant(ele->getLhs()) or this->isConstant(ele->getRhs())) {}
            else {
                functionCallFlag = true;
                XORGenModel(ele->getLhs(), ele->getRhs(), ele, false, true);
            }
        } else if (ele->getOp() == ASTNode::AND or ele->getOp() == ASTNode::OR) {
            functionCallFlag = true;
            ANDandORGenModel(ele->getLhs(), ele->getRhs(), ele, false, true);
        } else if (ele->getOp() == ASTNode::BOXOP) {
            functionCallFlag = true;
            if (ele->getLhs()->getNodeName().substr(0, 4) == "sbox") {
                // 不管是数组类型的sbox op或者uints类型的sbox op，都可以统一处理，因为我们在三地址转换的时候，
                // 所有的赋值语句都会生成对应的三地址，所以此时所有的boxop都会被统一直接处理，不会存在再symbolIndex类型中再进行
                // boxop的模型生成了
                SboxGenModel(ele->getLhs(), ele->getRhs(), ele, true);
            } else if (ele->getLhs()->getNodeName().substr(0, 4) == "pbox") {
                PboxGenModel(ele->getLhs(), ele->getRhs(), ele, true);
            }
        } else if (ele->getOp() == ASTNode::TOUINT) {
            // touint需要根据目前的ele名字拆分成为s个1bit的三地址实例，并根据touint的链接对象依次分配对应的xCounter
            if (ele->getLhs()->getOp() == ASTNode::BOXINDEX) {
                ThreeAddressNodePtr left = ele->getLhs();
                std::vector<int> leftIdx = extIdxFromTOUINTorBOXINDEX(left, false, true);
                int size = transNodeTypeSize(ele->getNodeType());
                for (int j = 0; j < size; ++j) {
                    this->kschdNameMxIndex[ele->getNodeName() + "_$B$_" + std::to_string(j)] = leftIdx[j];
                }
            }
        } else if (ele->getOp() == ASTNode::SYMBOLINDEX) {
            ThreeAddressNodePtr left = ele->getLhs();
            if (left->getOp() == ASTNode::BOXOP) {
                // box op 已经被处理
            } else
                assert(false);
        }
        else
            assert(false);


        if (!functionCallFlag) {
            if (ele->getLhs()->getNodeType() == NodeType::ARRAY and ele->getOp() == ASTNode::SYMBOLINDEX) {
                std::string finderName = ele->getLhs()->getNodeName() + "_$B$_" + ele->getRhs()->getNodeName();
                for (const auto &pair: this->kschdNameMxIndex) {
                    if (finderName == pair.first)
                        this->kschdNameMxIndex[ele->getNodeName()] = pair.second;
                }
            } else {
                for (const auto &pair: this->kschdNameMxIndex) {
                    if (ele->getLhs() != nullptr)
                        if (ele->getLhs()->getNodeName() == pair.first)
                            this->kschdNameMxIndex[ele->getNodeName()] = pair.second;
                    if (ele->getRhs() != nullptr)
                        if (ele->getRhs()->getNodeName() == pair.first)
                            this->kschdNameMxIndex[ele->getNodeName()] = pair.second;
                }
            }
        } else
            functionCallFlag = false;
    }

    // 现在this->sboxNameMxIndex找到return value对应的xCounter，然后再添加output对应xCounter的map至this->tanNameMxIndex
    for (const auto& rtn : procedureH->getReturns()) {
        std::string name = rtn->getNodeName();
        if (rtn->getOp() == ASTNode::SYMBOLINDEX) {
            name = rtn->getLhs()->getNodeName() + "_$B$_" + rtn->getRhs()->getNodeName();
        }
        for (const auto &pair: this->kschdNameMxIndex) {
            if (name == pair.first) {
                this->kschdRtnIdxSave.push_back(pair.second);
            }
        }
    }

    if (this->kschdRtnIdxSave.size() != input.size()) {
        std::cout << "the input size of key schedule is not equal to the output size !" << std::endl;
        // assert(false);
    }

    // debug
    /*std::cout << "\n ------------------------ key schedule all map index ------------------------ \n" << std::endl;
    for (auto p : this->kschdNameMxIndex) {
        std::cout << p.first << " : " << p.second << std::endl;
    }

    std::cout << "\n ------------------------ key schedule output index ------------------------ \n" << std::endl;
*/
    // 类似的，不同key schedule之间的建模是独立的，因此只需要根据kschdNameMxIndex中return的map来更新tanNameMxIndex的实际返回参数的map
    for (int i = 0; i < output.size(); ++i) {
        this->tanNameMxIndex["main_" + output[i]->getNodeName()] = this->kschdRtnIdxSave[i];
        //std::cout << "main_" + output[i]->getNodeName() << " : " << i << " : " << this->kschdRtnIdxSave[i] << std::endl;
    }
    this->kschdRtnIdxSave.clear();
}
    

void DiffRBMILP::sboxFunctionGenModel(const ProcedureHPtr &procedureH, std::vector<ThreeAddressNodePtr> input, std::vector<ThreeAddressNodePtr> output) {
    // 每次以函数调用的方式对sbox进行建模时，需要clear this->sboxNameMxIndex，
    // 因为此时的sbox建模本身是对同一个function进行多次调用，所以不能使两次调用之间产生冲突
    this->sboxNameMxIndex.clear();
    std::vector<int> inputIdx;
    // 每次sboxFunction的调用，其input的对应xCounter都由给定的实参对应的xCounter来决定
    int idx = 0;
    for (const auto & i : procedureH->getParameters().at(0)) {
        this->sboxNameMxIndex[i->getNodeName()] = this->tanNameMxIndex[input[idx]->getNodeName()];
        inputIdx.push_back(this->tanNameMxIndex[input[idx]->getNodeName()]);
        idx++;
    }

    // generate the constraints that bound if the sbox is active or not
    std::ofstream scons(this->modelPath, std::ios::app);
    if (!scons){
        std::cout << "Wrong file path ! " << std::endl;
    } else {
        for (int i = 0; i < inputIdx.size(); ++i) {
            if (i != inputIdx.size() - 1)
                scons << "x" << inputIdx[i] << " + ";
            else
                scons << "x" << inputIdx[i];
        }
        scons << " - A" << ACounter << " >= 0\n";
        for (auto i: inputIdx)
            scons << "A" << ACounter << " - x" << i << " >= 0\n";
        ACounter++;
    }

    bool functionCallFlag;
    for (int i = 0; i < procedureH->getBlock().size(); ++i) {
        functionCallFlag = false;
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);
        if (ele->getOp() == ASTNode::XOR) {
            // XOR with keys or constants does not affect differential propagation
            // based on our syntax, we only omit the XOR with constants
            if (this->isConstant(ele->getLhs()) or this->isConstant(ele->getRhs())) {}
            else {
                functionCallFlag = true;
                XORGenModel(ele->getLhs(), ele->getRhs(), ele, true, false);
            }
        } else if (ele->getOp() == ASTNode::AND or ele->getOp() == ASTNode::OR) {
            functionCallFlag = true;
            ANDandORGenModel(ele->getLhs(), ele->getRhs(), ele, true, false);
        } else if (ele->getOp() == ASTNode::TOUINT) {
            // touint需要根据目前的ele名字拆分成为s个1bit的三地址实例，并根据touint的链接对象依次分配对应的xCounter
            if (ele->getLhs()->getOp() == ASTNode::BOXINDEX) {
                ThreeAddressNodePtr left = ele->getLhs();
                std::vector<int> leftIdx = extIdxFromTOUINTorBOXINDEX(left, true, false);
                int size = transNodeTypeSize(ele->getNodeType());
                for (int j = 0; j < size; ++j) {
                    this->sboxNameMxIndex[ele->getNodeName() + "_$B$_" + std::to_string(j)] = leftIdx[j];
                }
            }
        } else
            assert(false);

        // If no function call is performed, it means that this node does not participate in any constraint generation operation.
        // At this time, if the left and right child nodes have been stored in xIndex, we need to overwrite their original nodes,
        // because subsequent operations need to be carried out on their basis.
        // Here we still use name of node to judge, because different pointers may point to the same memory address.
        if (!functionCallFlag) {
            if (ele->getLhs()->getNodeType() == NodeType::ARRAY and ele->getOp() == ASTNode::SYMBOLINDEX) {
                std::string finderName = ele->getLhs()->getNodeName() + "_$B$_" + ele->getRhs()->getNodeName();
                for (const auto &pair: this->sboxNameMxIndex) {
                    if (finderName == pair.first)
                        this->sboxNameMxIndex[ele->getNodeName()] = pair.second;
                }
            } else {
                for (const auto &pair: this->sboxNameMxIndex) {
                    if (ele->getLhs() != nullptr)
                        if (ele->getLhs()->getNodeName() == pair.first)
                            this->sboxNameMxIndex[ele->getNodeName()] = pair.second;
                    if (ele->getRhs() != nullptr)
                        if (ele->getRhs()->getNodeName() == pair.first)
                            this->sboxNameMxIndex[ele->getNodeName()] = pair.second;
                }
            }
        } else
            functionCallFlag = false;
    }

    // 现在this->sboxNameMxIndex找到return value对应的xCounter，然后再添加output对应xCounter的map至this->tanNameMxIndex
    std::vector<int> sboxRtnIdxSave;
    for (const auto& rtn : procedureH->getReturns()) {
        for (const auto &pair: this->sboxNameMxIndex) {
            if (rtn->getNodeName() == pair.first) {
                sboxRtnIdxSave.push_back(pair.second);
            }
        }
    }

    for (int i = 0; i < output.size(); ++i) {
        this->tanNameMxIndex[output[i]->getNodeName()] = sboxRtnIdxSave[i];
    }
}



void DiffRBMILP::XORGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
                             const ThreeAddressNodePtr &result, bool ifSboxFuncCall, bool ifKschdCall) {
    int inputIdx1 = 0, inputIdx2 = 0;
    if (ifSboxFuncCall) {
        for (const auto& pair : this->sboxNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    } else if (ifKschdCall) {
        for (const auto& pair : this->kschdNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    } else {
        for (const auto& pair : this->tanNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    }

    // 进行xor操作的被操作对象可能是类型为uints的三地址实例的某个bit
    if (ifSboxFuncCall) {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->sboxNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    } else if (ifKschdCall) {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->kschdNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->kschdNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->kschdNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->kschdNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    } else {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->tanNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    }


    if (ifSboxFuncCall) {
        if (inputIdx1 == 0) {
            inputIdx1 = this->xCounter;
            this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
            this->xCounter++;        }
        if (inputIdx2 == 0) {
            inputIdx2 = this->xCounter;
            this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
            this->xCounter++;
        }
    } else if (ifKschdCall) {
        if (inputIdx1 == 0) {
            inputIdx1 = this->xCounter;
            this->kschdNameMxIndex[left->getNodeName()] = inputIdx1;
            this->xCounter++;
        }
        if (inputIdx2 == 0) {
            inputIdx2 = this->xCounter;
            this->kschdNameMxIndex[left->getNodeName()] = inputIdx2;
            this->xCounter++;
        }
    } else {
        if (inputIdx1 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx1 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx2 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    }

    int outputIdx = this->xCounter;
    if (ifSboxFuncCall) {
        this->sboxNameMxIndex[result->getNodeName()] = outputIdx;
    } else if (ifKschdCall) {
        this->kschdNameMxIndex[result->getNodeName()] = outputIdx;
    } else {
        this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    }
    this->xCounter++;
    switch (this->xorConSel) {
        case 1:
            DiffMILPcons::bXorC1(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->dCounter);
            break;
        case 2:
            DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
            break;
        case 3:
            DiffMILPcons::bXorC3(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->dCounter);
            break;
        default:
            DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
            break;
    }
}


void DiffRBMILP::ANDandORGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
                                  const ThreeAddressNodePtr &result, bool ifSboxFuncCall, bool ifKschdCall) {
    int inputIdx1 = 0, inputIdx2 = 0;
    if (ifSboxFuncCall) {
        for (const auto& pair : this->sboxNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    } else if (ifKschdCall) {
        for (const auto& pair : this->kschdNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    } else {
        for (const auto& pair : this->tanNameMxIndex) {
            if (pair.first == left->getNodeName())
                inputIdx1 = pair.second;
            if (pair.first == right->getNodeName())
                inputIdx2 = pair.second;
        }
    }

    // 进行xor操作的被操作对象可能是类型为uints的三地址实例的某个bit
    if (ifSboxFuncCall) {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->sboxNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    } else if (ifKschdCall) {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->kschdNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->kschdNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->kschdNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->kschdNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    } else {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }

        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->tanNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    }


    if (ifSboxFuncCall) {
        if (inputIdx1 == 0) {
            inputIdx1 = this->xCounter;
            this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
            this->xCounter++;
        }
        if (inputIdx2 == 0) {
            inputIdx2 = this->xCounter;
            this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
            this->xCounter++;
        }
    } else if (ifKschdCall) {
        if (inputIdx1 == 0) {
            inputIdx1 = this->xCounter;
            this->kschdNameMxIndex[left->getNodeName()] = inputIdx1;
            this->xCounter++;
        }
        if (inputIdx2 == 0) {
            inputIdx2 = this->xCounter;
            this->kschdNameMxIndex[left->getNodeName()] = inputIdx2;
            this->xCounter++;
        }
    } else {
        if (inputIdx1 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx1 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx2 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    }

    int outputIdx = this->xCounter;
    if (ifSboxFuncCall) {
        this->sboxNameMxIndex[result->getNodeName()] = outputIdx;
    } else if (ifKschdCall) {
        this->kschdNameMxIndex[result->getNodeName()] = outputIdx;
    } else {
        this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    }
    this->xCounter++;
    if (mode == "AS")
        DiffMILPcons::bAndOrC(this->modelPath, inputIdx1, inputIdx2, outputIdx);
    else if (mode == "DC")
        DiffMILPcons::dAndOrC(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->PCounter);
    else
        assert(false);
}


void DiffRBMILP::SboxGenModel(const ThreeAddressNodePtr &sbox, const ThreeAddressNodePtr &input,
                              const ThreeAddressNodePtr &output, bool ifKschdCall) {
    // Determine the input variable of the sbox generation constraint based on the type of input
    std::vector<int> inputIdx = extIdxFromTOUINTorBOXINDEX(input, false, ifKschdCall);
    int outputSize = sboxOutputSize[sbox->getNodeName()];
    std::vector<int> outputIdx;
    for (int i = 0; i < outputSize; ++i) {
        outputIdx.push_back(this->xCounter);
        if (ifKschdCall) {
            this->kschdNameMxIndex[output->getNodeName() + "_$B$_" + std::to_string(i)] = this->xCounter;
        } else {
            this->tanNameMxIndex[output->getNodeName() + "_$B$_" + std::to_string(i)] = this->xCounter;
        }
        this->xCounter++;
    }

    if (this->mode == "AS") {
        switch (this->sboxConSel) {
            case 1:
                DiffMILPcons::bSboxC1(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->branchNumMin[sbox->getNodeName()], this->dCounter, this->ACounter, sboxIfInjective[sbox->getNodeName()]);
                break;
            case 2:
                DiffMILPcons::bSboxC2(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->ACounter, sboxIfInjective[sbox->getNodeName()]);
                break;
            case 3:
                DiffMILPcons::bSboxC3(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->ACounter);
                break;
            default:
                DiffMILPcons::bSboxC2(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->ACounter, sboxIfInjective[sbox->getNodeName()]);
        }
    } else if (this->mode == "DC") {
        switch (this->sboxConSel) {
            case 1:
                DiffMILPcons::dSboxC1(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->branchNumMin[sbox->getNodeName()], this->dCounter, this->PCounter, sboxIfInjective[sbox->getNodeName()], this->sboxExtWeighted.size());
                break;
            case 2:
                DiffMILPcons::dSboxC2(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->PCounter, sboxIfInjective[sbox->getNodeName()], this->sboxExtWeighted.size());
                break;
            case 3:
                DiffMILPcons::dSboxC3(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->PCounter, this->sboxExtWeighted.size());
                break;
            default:
                DiffMILPcons::dSboxC2(this->modelPath, inputIdx, outputIdx, this->sboxIneqs[sbox->getNodeName()],
                                      this->PCounter, sboxIfInjective[sbox->getNodeName()], this->sboxExtWeighted.size());
        }
    } else
        assert(false);
}

void DiffRBMILP::PboxGenModel(const ThreeAddressNodePtr &pbox, const ThreeAddressNodePtr &input,
                              const ThreeAddressNodePtr &output, bool ifKschdCall) {
    std::vector<int> pboxValue = this->Box[pbox->getNodeName()];
    // pboxGenModel只会在round function中被调用
    std::vector<int> inputIdx = extIdxFromTOUINTorBOXINDEX(input, false, ifKschdCall);
    std::vector<int> outputIdx;
    for (int i = 0; i < pboxValue.size(); ++i)
        outputIdx.push_back(0);
    for (int i = 0; i < pboxValue.size(); ++i)
        //outputIdx[pboxValue.size() - 1 - pboxValue[pboxValue.size() - 1 - i]] = inputIdx[i];
        //outputIdx[pboxValue[i]] = inputIdx[i];
        outputIdx[i] = inputIdx[pboxValue[i]];
    for (int i = 0; i < outputIdx.size(); ++i)
        if (!ifKschdCall)
            this->tanNameMxIndex[output->getNodeName() + "_$B$_" + std::to_string(i)] = outputIdx[i];
        else
            this->kschdNameMxIndex[output->getNodeName() + "_$B$_" + std::to_string(i)] = outputIdx[i];
}


void DiffRBMILP::MatrixVectorGenModel(const ThreeAddressNodePtr &pboxm, const ThreeAddressNodePtr &input,
                                      const vector<ThreeAddressNodePtr> &output) {
    /*
     * 这里我们梳理一下整个矩阵乘法的处理流程：
     *
     * 1. 我们描述的应该是所有输入和所有输出之间的差分传播关系，因为输出 vector 中的每个 y_i 之间的结果互相并不影响，
     *    所以我们可以单独考虑每个 y_i 的传播过程;
     *
     * 2. 输出vector的每个元素 y_i 应该是输入vector中所有 x_j 与对应第 i 行的每个元素 M_ij 依次进行有限域乘法的结
     *    果的异或， 因此，对于每个输出元素 y_i，都要去搜集影响其映射的输入元素 x_j;
     *
     * 3. 对于每个影响输出 y_i 的输入元素 x_j， 其都要先与矩阵 M 中的元素 M_ij 进行有限域的乘法，我们需要将该乘法转
     *    换乘 移位 加 异或，取模 的运算， 与 x_j 与 M_ij 的计算结果之间再两两进行异或，得到 y_i;
     *
     * 4. 某个有限域乘法中，模的结果就是 2^(n-1) * 2 的值， 如： AES的模就是 10000000 * 00000010
     *
     * 实现逻辑：
     * 1. 对于每个 y_i， 维护一个 map， 其保存正对应 y_i 的所有 x_j;
     *
     * 2. 根据 x_j 对应操作的 M_ij, 将有限域乘法转换成 移位，异或，取模（异或模）的操作， 并将所有需要进行异或的值，
     *    用一个map保存起来，映射到 y_i.
     *    此时， 每个 y_i 应该都映射到了其对应所有 x_j 下，生成的需要两两进行异或的所有值.
     *
     *    注：此时的操作需要区分 uint1 与 uints：
     *      1） 对于 uint1，可以直接根据常数项是否为0来判断 y_i 映射到的 x_j 集合
     *      2） 对于 uints, 需要根据与常数项有限域乘法的拆分结果，来得到 y_i 映射到的所有集合元素， 当某个常数项不为
     *          0时，对应 x_j 应该会生成一个或多个元素（y_i的映射集合中的元素）
     *
     * 3. 根据初始输入 x_j 和 最终输出 y_i 构造约束
     *
     * */

    // 提取所有input
    std::vector<ThreeAddressNodePtr> inputTAN;
    if (input->getOp() == ASTNode::BOXINDEX) {
        ThreeAddressNodePtr left = input;
        while (left->getOp() == ASTNode::BOXINDEX) {
            inputTAN.push_back(left->getLhs());
            left = left->getRhs();
        }
        inputTAN.push_back(left);
    } else
        assert(false);

    // 初始化 matrix 和 ffm, 这里应该直接吧 matrix 和 ffm 直接用二进制字符串表示
    std::vector<std::vector<int>> Matrix; // matrix
    std::vector<int> pboxmValue = pboxM[pboxm->getNodeName()];
    int rowSize = inputTAN.size(), rowCounter = 0;
    std::vector<int> tempRow;
    // Reassemble pboxm into a matrix
    for (int & i : pboxmValue) {
        tempRow.push_back(i);
        rowCounter++;
        if (rowCounter == rowSize) {
            Matrix.push_back(tempRow);
            tempRow.clear();
            rowCounter = 0;
        }
    }
    tempRow.clear();
    rowCounter = 0;
    std::vector<int> ffmValue = Ffm[pboxm->getNodeName()];
    std::vector<std::vector<int>> Ffm; // ffm
    int ffmRowSize = int(sqrt(ffmValue.size())); // 因为ffm一定是个方阵，所以求平方根以后就是ffm每行的size
    for (int & i : ffmValue) {
        tempRow.push_back(i);
        rowCounter++;
        if (rowCounter == ffmRowSize) {
            Ffm.push_back(tempRow);
            tempRow.clear();
            rowCounter = 0;
        }
    }

    // 区分 uint1 和 uints
    int eleSize;
    if (inputTAN[0]->getNodeType() == NodeType::PARAMETER)
        eleSize = 1;
    else
        eleSize = transNodeTypeSize(inputTAN[0]->getNodeType());

    // 根据 elesize 分别进行处理
    // uints
    if (eleSize != 1) {
        // 二进制字符串表示数组
        std::vector<std::vector<std::string>> MatrixStr;
        for (const auto& row : Matrix) {
            std::vector<std::string> tt;
            for (auto ele: row) {
                // 先将FFm中的每个元素转化为长度为sizeS的二进制字符串
                std::string eleB = std::to_string(utilities::d_to_b(ele));
                for (int i = eleB.size(); i < eleSize; ++i)
                    eleB = "0" + eleB;
                tt.push_back(eleB);
            }
            MatrixStr.push_back(tt);
        }
        // 有限域的模
        int module = Ffm[2][int(pow(2, (eleSize - 1)))];
        std::string moduleStr = std::to_string(utilities::d_to_b(module));
        for (int i = moduleStr.size(); i < eleSize; ++i)
            moduleStr = "0" + moduleStr;
        std::vector<int> moduleVec;
        for (char i : moduleStr) {
            if (i == '0') moduleVec.push_back(0);
            else if (i == '1') moduleVec.push_back(1);
        }

        std::map<ThreeAddressNodePtr, std::vector<ThreeAddressNodePtr>> MoutMap; // y_i -> x_j
        std::map<ThreeAddressNodePtr, std::vector<std::string>> MoutMapMatrix; // y_i -> M_ij
        // 有限域乘法描述约束映射
        for (int i = 0; i < output.size(); ++i) {
            std::vector<ThreeAddressNodePtr> MoutEleMap;
            std::vector<std::string> MoutEleMapMatrix;
            for (int j = 0; j < rowSize; ++j) {
                MoutEleMap.push_back(inputTAN[j]);
                MoutEleMapMatrix.push_back(MatrixStr[i][j]);
            }
            MoutMap[output[i]] = MoutEleMap;
            MoutMapMatrix[output[i]] = MoutEleMapMatrix;
        }

        std::map<ThreeAddressNodePtr, std::vector<int>> MoutIdxMap; // y_i -> indexes
        std::map<ThreeAddressNodePtr, std::vector<int>> MintIdxMap; // x_j -> indexes
        for (auto ele : inputTAN) {
            std::vector<int> inputIdx = extIdxFromTOUINTorBOXINDEX(ele, false, false);
            MintIdxMap[ele] = inputIdx;
        }
        for (auto ele : output) {
            std::vector<int> outputIdx;
            for (int i = 0; i < eleSize; ++i) {
                this->tanNameMxIndex[ele->getNodeName() + "_$B$_" + std::to_string(i)] = this->xCounter;
                outputIdx.push_back(this->xCounter);
                this->xCounter++;
            }
            MoutIdxMap[ele] = outputIdx;
        }

        // 构造每个 y_i 对应所有 x_j 下，需要两两进行异或的所有值的映射
        std::map<std::vector<int>, std::vector<std::vector<int>>> MoutIdxIntIdxes;
        // 每个 y_i 对应下可能的溢出输入位
        std::map<std::vector<int>, std::vector<int>> MoutIdxOverflowIdxes;

        auto item = MoutMap.begin();
        auto itemMoutMatrix = MoutMapMatrix.begin();
        for (int i = 0; i < MoutMap.size(); ++i) {
            // 每个 y_i 映射的所有值
            std::vector<std::vector<int>> tMapIdxes;
            std::vector<int> tOverflows; // 可能溢出的 x_j 中的某一位, 这里直接保存对应的index
            // 对 matrix 中每一行的元素，都应该与 MoutMap 中对应的元素一一对应
            for (int j = 0; j < itemMoutMatrix->second.size(); ++j) {
                std::vector<int> x_i = MintIdxMap[item->second[j]];
                std::string M_ij = itemMoutMatrix->second[j];
                int shiftNum = 0; // 移位数量

                for (int k = 0; k < eleSize; ++k) {
                    // 对于每个 M_ij，其每一位都是确定的，可以用来判断对应的 x_j 移位数目，但是如何判断 x_j 移位后是否溢出呢？
                    // 尝试：
                    // gurobi 支持 indicator constraint, 可以使用左移移出的 x_jk 作为表示 binary，用于标明是否要异或 模向量
                    if (M_ij[eleSize - 1 - k] == '1') {
                        // 这里添加可能溢出位时还要判断是否里面已经有了，如果有该位，则需要消除。
                        // 因为与模的偶数次异或会抵消
                        if (k > 0) {
                            for (int l = 0; l < k; ++l) {
                                auto finder = std::find(tOverflows.begin(), tOverflows.end(), x_i[l]);
                                if (finder != tOverflows.end())
                                    tOverflows.erase(finder);
                                else
                                    tOverflows.push_back(x_i[l]);
                            }

                            // 这里应该给一个用于shift的函数，接受一个vector和移位的数目，返回移位后的vector
                            tMapIdxes.push_back(shiftVec(x_i, k));
                        } else {
                            // 不用移位，直接放入 y_i 对应映射
                            tMapIdxes.push_back(x_i);
                        }
                    } else if (M_ij[eleSize - 1 - k] == '0')
                        continue;
                    shiftNum++;
                }
            }
            MoutIdxIntIdxes[MoutIdxMap[item->first]] = tMapIdxes;
            MoutIdxOverflowIdxes[MoutIdxMap[item->first]] = tOverflows;
            item++;
            itemMoutMatrix++;
        }

        // 提取出每个 y_i 映射的 xor 操作对象集合， 和 y_i 映射的溢出位集合，
        // 根据两个集合生成约束

        // 需要在 溢出位集合中所有元素异或结果为1时， 增加和 模 的异或
        // 新建一个变量 f 作为溢出位异或的结果，然后根据 f 的值为 0 还是 1 来判断是否要和 模 进行异或, 然后输出最终结果 y_end
        // if f == 0 : y_end = y_i    ( y_end - y_i = 0 )
        // if f == 1 : y_end = - y_i    ( y_end + y_i = 0 )
        auto itemMidx = MoutIdxIntIdxes.begin();
        auto itemMove = MoutIdxOverflowIdxes.begin();
        for (int i = 0; i < MoutIdxIntIdxes.size(); ++i) {
            if (matrixConSel == 1 or matrixConSel == 2) {
                DiffMILPcons::bMatrixEntryC12(this->modelPath, itemMidx->second, itemMidx->first, itemMove->second, moduleVec,
                                              this->xCounter, this->yCounter, this->fCounter, this->dCounter, this->matrixConSel);
            } else if (matrixConSel == 2) {
                DiffMILPcons::bMatrixEntryC3(this->modelPath, itemMidx->second, itemMidx->first, itemMove->second, moduleVec,
                                             this->xCounter, this->yCounter, this->fCounter);
            }
            itemMidx++;
            itemMove++;
        }
    } else {
        // 对 uint1 的处理需要延用之前的方法
        // rowSize即矩阵的列数，最终矩阵乘法的结果是 和列数相同数目的矩阵 按相对应位进行XOR，所以我们需要先得到rowSize数目的vector，其中每个元素都是最终要进行XOR的vector
        std::map<ThreeAddressNodePtr, std::vector<ThreeAddressNodePtr>> MoutMap;
        for (int i = 0; i < output.size(); ++i) {
            std::vector<ThreeAddressNodePtr> MoutEleMap;
            for (int j = 0; j < rowSize; ++j) {
                if (Ffm[Matrix[i][j]][1] == 1) {
                    MoutEleMap.push_back(inputTAN[j]);
                }
            }
            MoutMap[output[i]] = MoutEleMap;
        }

        for (auto pair : MoutMap) {
            if (pair.second.size() >= 2) {
                if (this->matrixConSel == 1 or this->matrixConSel == 2) {
                    multiXORGenModel12(pair.second, pair.first);
                } else if (this->matrixConSel == 3) {
                    multiXORGenModel3(pair.second, pair.first);
                }
            }
            else {
                int rrr = this->tanNameMxIndex[pair.second.at(0)->getNodeName()];
                this->tanNameMxIndex[pair.first->getNodeName()] = this->tanNameMxIndex[pair.second.at(0)->getNodeName()];
            }
        }
    }
}


void DiffRBMILP::multiXORGenModel12(std::vector<ThreeAddressNodePtr> input, ThreeAddressNodePtr result) {
    ThreeAddressNodePtr input1 = input[0], input2 = input[1];
    int inputIdx1 = 0, inputIdx2 = 0;
    for (const auto& pair : this->tanNameMxIndex) {
        if (pair.first == input1->getNodeName())
            inputIdx1 = pair.second;
        if (pair.first == input2->getNodeName())
            inputIdx2 = pair.second;
    }
    if (inputIdx1 == 0) {
        if (!this->rtnIdxSave.empty()) {
            inputIdx1 = this->rtnIdxSave.front();
            this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            this->tanNameMxIndex[input1->getNodeName()] = inputIdx1;
        } else {
            inputIdx1 = this->xCounter;
            this->tanNameMxIndex[input1->getNodeName()] = inputIdx1;
            this->xCounter++;
        }
    }
    if (inputIdx2 == 0) {
        if (!this->rtnIdxSave.empty()) {
            inputIdx2 = this->rtnIdxSave.front();
            rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            this->tanNameMxIndex[input2->getNodeName()] = inputIdx2;
        } else {
            inputIdx2 = this->xCounter;
            this->tanNameMxIndex[input2->getNodeName()] = inputIdx2;
            this->xCounter++;
        }
    }

    int outputIdx = this->xCounter;
    this->xCounter++;
    if (input.size() == 2)
        this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    switch (matrixConSel) {
        case 1:
            DiffMILPcons::bXorC1(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->dCounter);
            break;
        case 2:
            DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
            break;
        default:
            DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
    }

    // Process the remaining elements
    if (input.size() > 2) {
        for (int i = 2; i < input.size(); ++i) {
            inputIdx1 = outputIdx;
            ThreeAddressNodePtr inputNew = input[i];
            for (const auto& pair : this->tanNameMxIndex)
                if (pair.first == inputNew->getNodeName())
                    inputIdx2 = pair.second;

            if (inputIdx2 == 0) {
                if (!this->rtnIdxSave.empty()) {
                    inputIdx2 = this->rtnIdxSave.front();
                    this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                    this->tanNameMxIndex[inputNew->getNodeName()] = inputIdx2;
                } else {
                    inputIdx2 = this->xCounter;
                    this->tanNameMxIndex[inputNew->getNodeName()] = inputIdx2;
                    this->xCounter++;
                }
            }

            outputIdx = this->xCounter;
            this->xCounter++;
            if (i == input.size() - 1)
                this->tanNameMxIndex[result->getNodeName()] = outputIdx;
            switch (matrixConSel) {
                case 1:
                    DiffMILPcons::bXorC1(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->dCounter);
                    break;
                case 2:
                    DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
                    break;
                default:
                    DiffMILPcons::bXorC2(this->modelPath, inputIdx1, inputIdx2, outputIdx);
            }
        }
    }
}


void DiffRBMILP::multiXORGenModel3(std::vector<ThreeAddressNodePtr> input, ThreeAddressNodePtr result) {
    std::vector<int> inputIdx;
    for (auto ele : input) {
        bool finder = false;
        for (const auto& pair : this->tanNameMxIndex) {
            if (pair.first == ele->getNodeName()) {
                inputIdx.push_back(pair.second);
                finder = true;
            }
        }
        if (!finder) {
            inputIdx.push_back(0);
        }
    }

    for (int i = 0; i < inputIdx.size(); ++i) {
        if (inputIdx[i] == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx[i] = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[input[i]->getNodeName()] = inputIdx[i];
            } else {
                inputIdx[i] = this->xCounter;
                this->tanNameMxIndex[input[i]->getNodeName()] = inputIdx[i];
                this->xCounter++;
            }
        }
    }

    int outputIdx = this->xCounter;
    this->xCounter++;
    this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    DiffMILPcons::bNXorC3(this->modelPath, inputIdx, outputIdx, this->yCounter);
}


void DiffRBMILP::ADDandMINUSGenModel(vector<ThreeAddressNodePtr> &input1, vector<ThreeAddressNodePtr> &input2,
                                     const vector<ThreeAddressNodePtr> &output, int AddOrMinus, bool ifSboxFuncCall, bool ifKschdCall) {
    std::vector<int> inputIdx1, inputIdx2;
    std::vector<std::vector<ThreeAddressNodePtr>> input = {input1, input2};
    if (ifSboxFuncCall) {
        for (auto in: input1) {
            if (this->sboxNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx1.push_back(this->sboxNameMxIndex.find(in->getNodeName())->second);
            } else {
                this->sboxNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx1.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    } else if (ifKschdCall) {
        for (auto in: input1) {
            if (this->kschdNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx1.push_back(this->kschdNameMxIndex.find(in->getNodeName())->second);
            } else {
                this->kschdNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx1.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    } else {
        for (auto in: input1) {
            if (this->tanNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx1.push_back(this->tanNameMxIndex.find(in->getNodeName())->second);
            } else if (!this->rtnIdxSave.empty()) {
                this->tanNameMxIndex[in->getNodeName()] = this->rtnIdxSave.front();
                inputIdx1.push_back(this->rtnIdxSave.front());
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            } else {
                this->tanNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx1.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    }

    if (ifSboxFuncCall) {
        for (auto in: input2) {
            if (this->sboxNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx2.push_back(this->sboxNameMxIndex.find(in->getNodeName())->second);
            } else {
                this->sboxNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx2.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    } else if (ifKschdCall) {
        for (auto in: input2) {
            if (this->kschdNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx1.push_back(this->kschdNameMxIndex.find(in->getNodeName())->second);
            } else {
                this->kschdNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx1.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    } else {
        for (auto in: input2) {
            if (this->tanNameMxIndex.count(in->getNodeName()) != 0) {
                inputIdx2.push_back(this->tanNameMxIndex.find(in->getNodeName())->second);
            } else if (!this->rtnIdxSave.empty()) {
                this->tanNameMxIndex[in->getNodeName()] = this->rtnIdxSave.front();
                inputIdx2.push_back(this->rtnIdxSave.front());
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
            } else {
                this->tanNameMxIndex[in->getNodeName()] = this->xCounter;
                inputIdx2.push_back(this->xCounter);
                this->xCounter++;
            }
        }
    }

    std::vector<int> outputIdx;
    if (ifSboxFuncCall) {
        for (const auto& ele : output) {
            this->sboxNameMxIndex[ele->getNodeName()] = this->xCounter;
            outputIdx.push_back(this->xCounter);
            this->xCounter++;
        }
    } else if (ifKschdCall) {
        for (const auto& ele : output) {
            this->sboxNameMxIndex[ele->getNodeName()] = this->xCounter;
            outputIdx.push_back(this->xCounter);
            this->xCounter++;
        }
    } else {
        for (const auto& ele : output) {
            this->tanNameMxIndex[ele->getNodeName()] = this->xCounter;
            outputIdx.push_back(this->xCounter);
            this->xCounter++;
        }
    }

    if (this->mode == "AS") {
        // Add
        if (AddOrMinus == 1)
            DiffMILPcons::bAddC(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->ARXineqs, dCounter, PCounter);
            // Minus
        else if (AddOrMinus == 2)
            // a - b = c   ->   a = b + c   ->   b + c = a
            DiffMILPcons::bAddC(this->modelPath, outputIdx, inputIdx2, inputIdx1, this->ARXineqs, dCounter, PCounter);
        else
            assert(false);
    } else if (this->mode == "DC") {
        // Add
        if (AddOrMinus == 1)
            DiffMILPcons::dAddC(this->modelPath, inputIdx1, inputIdx2, outputIdx, this->ARXineqs, dCounter, PCounter);
            // Minus
        else if (AddOrMinus == 2)
            // a - b = c   ->   a = b + c   ->   b + c = a
            DiffMILPcons::dAddC(this->modelPath, outputIdx, inputIdx2, inputIdx1, this->ARXineqs, dCounter, PCounter);
        else
            assert(false);
    } else
        assert(false);
}
