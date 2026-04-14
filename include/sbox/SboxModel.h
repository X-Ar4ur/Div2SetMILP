//
// Created by Septi on 8/29/2022.
//

#ifndef EASYBC_SBOXMODEL_H
#define EASYBC_SBOXMODEL_H

#include <cassert>
#include <utility>
#include <cmath>
#include <bitset>
#include <cstdlib>
#include "SboxExtC.h"


/*
 * The class is for modeling a given S-box.
 * Modeling should first determine whether the goal of the overall MILP modeling is for
 * differential characteristic (DC), number of active S-box (AS) or ARX-based S-box (ARX).
 *
 * 1) construct DDT
 * 2) generate inequalities for modeling possible differential propagations
 * 3) calculate the weighted of extended bits for DC mode
 *
 * */

class SboxM {

private:
    SboxM() = default;
    std::string name_;
    std::vector<int> sbox_;
    // cryptanalysis : "differential" | "linear"
    std::string cryptanalysis_;
    // modeling mode: "AS" | "DC"
    std::string mode_;


    // used to record the number of different probabilities when mode_ == "DC"
    int ddtAndLatProbNum_{};
    std::vector<int> ddtAndLatProb_{};
    std::vector<float> ddtAndLatExtWeighted_{};
    // encode all probabilities of DDT and LAT
    std::unordered_map<std::string, std::string> ddtAndLatProbEncode_;

    // the path of ddt & lat
    std::string path_ = std::string(DPATH) + "sbox/"; // sbox数据存储的根目录
    std::string ddtAndLatPath_{}, arxPath_{}; // ARX 的建模是统一的，直接进行处理就行

    // impossible & possible differential propagation or linear mask
    std::vector<std::string> impossPm_;
    std::vector<std::string> possPm_;

    // input of sagemath
    std::string sageInPath_;
    std::string sageOutPath_;
    // output inequalities of sagemath
    std::vector<std::vector<int>> sageIneqs_;

    int sboxMDP;

    static std::string pointToBinaryString(const std::vector<int>& point);
    static std::string ensureTrailingSlash(std::string path);
    void initSagePaths(const std::string& basePath, int bitSize);
    void buildImpossFromPossPoints();
    void buildIneqsFromPossPoints(const std::vector<std::string>& possPoints);

public:
    SboxM(std::string name, std::vector<int> sbox, std::string cryptanalysis,std::string mode);
    // for arx
    SboxM(std::string name, std::string mode);
    static SboxM fromPointSet(std::string name,
                              std::vector<std::vector<int>> possPoints,
                              std::string analysisTag,
                              std::string outputRoot);

    void ddt_gen();
    void lat_gen();
    static int binCounter1(int input);

    void get_ext();     // for DC, calculate the number of extended bits
    void pattern_ext();     // extract impossible patterns and possible patterns
    void sage_ext();

    void arxDiff();
    void arxLinear();
    void arxSageGen();

    void trueTableGen();

    // used for superball
    // transform possible propagation from binary to
    void possBintoHex();

    void set_name(std::string name) { this->name_ = name;}
    std::string get_name() {return name_;}
    std::string get_ddtAndLat() {return ddtAndLatPath_;}
    std::string get_path() {return path_;}
    std::string get_mode() {return mode_;}
    int get_sbox_len() {
        if (!sbox_.empty())
            return int(log2(sbox_.size()));
        if (!possPm_.empty())
            return int(possPm_[0].size() / 2);
        return 0;
    }
    std::vector<std::string> get_poss() {return possPm_;}
    std::vector<std::string> get_imposs() {return impossPm_;}
    std::vector<std::vector<int>> get_sage_ineqs() {return sageIneqs_;}
    int get_dim() {return possPm_.empty() ? 0 : int(possPm_[0].size() + 1);} // get dimensionality
    int get_ext_len() {return ddtAndLatProbNum_;}

    // 用于结合keysize来对block cipher的安全性进行衡量
    float getSboxMDP() {
        std::sort(ddtAndLatProb_.rbegin(), ddtAndLatProb_.rend());
        // 真正的MDP应该是ddtProb中的第二个元素，因为第一个元素是0->0的对应概率值，即概率为1
        int maxProb = ddtAndLatProb_[1];
        //我们需要返回对应的log取值的相反数
        float rtn = -log2(maxProb / pow(2, sqrt(sbox_.size())));
        return  rtn;
    }

    // when we return the extWeighted_,
    // the "float" type should be transformed to "int" type
    std::vector<int> get_extWeighted() {
        std::vector<int> rtn;
        std::vector<float> temp = ddtAndLatExtWeighted_;
        while (true) {
            int times = 10, counter = 0;
            bool allInt = false;
            for (auto &weight : temp) {
                // weight *= times;
                if (weight - int(weight) == 0) {
                    counter++;
                }
            }
            if (counter == ddtAndLatExtWeighted_.size())
                break;
        }
        for (auto item : temp) {
            rtn.push_back(int(item));
        }
        return rtn;
    }
};

#endif //EASYBC_SBOXMODEL_H
