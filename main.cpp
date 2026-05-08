#include <iostream>
#include <chrono>
#include <ASTNode.h>
#include "Value.h"
#include "Interpreter.h"
#include "Transformer.h"
#include "differential/DiffSBMILP.h"
#include "differential/DiffSWMILP.h"
#include "differential/DiffRBMILP.h"
#include "differential/DiffRWMILP.h"
#include "linear/LinearSBMILP.h"
// #include "interpreter/semanticInter.h"
#include "SboxDivTrails.h"
#include "DivTrailsModel.h"
#include "division/Div2SetMILP.h"


extern int yyparse();
extern int yydebug;
extern FILE *yyin;
extern int yylineno;
extern std::shared_ptr<ASTNode::NBlock> programRoot;

int ASTNodeCounter = 0;

std::map<std::string, std::vector<int>> allBox = {};
std::map<std::string, std::vector<int>> pboxM = {};
std::map<std::string, int> pboxMSize = {};
std::map<std::string, std::vector<int>> Ffm = {};
std::string cipherName;

void SboxModelingMGR(std::vector<std::string> params);
void MILPMGR(std::vector<std::string> params);
void DivTrailsMGR(std::vector<std::string> params);

int main(int argc, const char* argv[]) {
    std::vector<std::string> params;
    for (int i = 1; i < argc; ++i) {
        params.emplace_back(argv[i]);
    }
    // sbox modeling
    if (argc > 1 and params[0] == "-div") {
        std::vector<std::string> divParams(params.begin() + 1, params.end());
        DivTrailsMGR(divParams);
    } else if (argc == 6) {
        SboxModelingMGR(params);
    } else if (argc >= 8) {
        MILPMGR(params);
    } else {
        params.clear();
        //std::string path = "../parametersSboxDemo.txt";
        std::string path = "../parametersMILPDemo.txt";
        std::ifstream file;
        file.open(path);
        std::string model, line;
        std::string whiteSpaces = " \n\r\t\f\v";
        while (getline(file, line)) {
            // trim and save parameters
            size_t first_non_space = line.find_first_not_of(whiteSpaces);
            line.erase(0, first_non_space);
            size_t last_non_space = line.find_last_not_of(whiteSpaces);
            line.erase(last_non_space + 1);
            params.push_back(line);
        }
        file.close();
        int paramNum = params.size();
        if (paramNum == 5) {
            SboxModelingMGR(params);
        } else if (paramNum >= 7) {
            MILPMGR(params);
        } else
            assert(false);
    }

    return 0;
}


void SboxModelingMGR(std::vector<std::string> params) {
    /*
     * argv[1] -> sboxName;
     * argv[2] -> sbox;    e.g.   "1,2,3,4,5,6,7,8,9,...,16"
     * argv[3] -> cryptanalysis choose
     *            cryptanalysis = "differential" -> differential cryptanalysis
     *            cryptanalysis = "linear" -> linear cryptanalysis
     * argv[4] -> modeling mode;
     *            mode = "AS" -> modeling differential propagation
     *            mode = "DC" -> modeling differential propagation with probability
     * argv[5] -> reduction methods;
     *            Note : here we only recommend the value of redMd takes 1 to 6;
     *            redMd = 1 -> greedy algorithm
     *            redMd = 2 -> sub_milp
     *            redMd = 3 -> convex_hull_tech
     *            redMd = 4 -> logic_cond
     *            redMd = 5 -> comb233
     *            redMd = 6 -> superball
     *            redMd = 7 -> read inequalities from external used to extract the results of method of Udovenko (Hellman) or CNF
     *            redMd = 8 -> read inequalities from external used to extract the results of method of Udovenko (Hellman) or CNF
     * */
    std::string sboxName = params[0];
    std::vector<string> sboxStr = utilities::split(params[1], ",");
    std::vector<int> sbox;
    for (const auto &ele: sboxStr) { sbox.push_back(std::stoi(ele)); };
    std::string cryptanalysis = params[2];
    std::string mode = params[3];
    int redMd = std::stoi(params[4]);
    SboxM sboxM(sboxName, sbox, cryptanalysis, mode);

    // 暂时在 differential 时才进行 inequalities 的 reduction
    // 对于 linear 下，我们也需要，但是后续还有概率建模时需要考虑，因此暂时不进行reduction。
    // 需要等到整体 sboxM 整合完毕
    if (cryptanalysis == "differential")
        std::vector<std::vector<int>> redResults = Red::reduction(redMd, sboxM);
}

void MILPMGR(std::vector<std::string> params) {
    /*
     * argv[1] -> the number of parameters
     * argv[2] -> EasyBC implementation file path;
     * argv[3] -> "differential" or "linear"
     * argv[4] -> "singleKey" or "relatedKey"
     * argv[5] -> word-wise or bit-wise or extended bit-wise
     *            techChoose = "w" -> word-wise
     *            techChoose = "b" -> bit-wise
     *            techChoose = "d" -> extended bit-wise
     *            the modeling mode for S-boxes, same as the "case 1", is depended on "w", "b" or "d"
     * argv[6] -> "cryptanalysis" or "evaluation"
     * argv[7] -> reduction methods for S-boxes; same as the "case 1";
     *
     * optional parameters :
     * argv[8]/argv[10]/argv[12]/argv[14]/argv[16]/argv[18]/
     *            -> startRound or allRounds or timer(second) or threadsNum or keySize or totalRoundNum
     * argv[9]/argv[11]/argv[13]/argv[15]/argv[17]/argv[19]/
     *            -> startRound or allRounds or timer(second) or threadsNum or keySize or totalRoundNum
     * */
    std::string filePath = params[1];
    yyin = fopen(filePath.c_str(), "r");
    if (!yyin) {
        std::cout << "Wrong Path : \n" << filePath << std::endl;
        assert(false);
    }
    yydebug = 0;
    yylineno = 1;
    std::vector<ProcValuePtr> res;
    Interpreter interpreter;
    if (!yyparse()) {
        std::cout << "Parsing complete\n" << std::endl;
    } else {
        std::cout << "Hint : wrong syntax at line " << yylineno << std::endl;
        assert(false);
    }
    interpreter.generateCode(*programRoot);
    res = interpreter.getProcs();

    std::cout << "ASTNodeCounter : " << ASTNodeCounter << std::endl;

    Transformer transformer(res);
    transformer.transformProcedures();

    // extract optional parameters
    int startRound = 1, endRound = 5, timer = 3600 * 24, threadsNum = 16, keySize = 0, totalRoundNum = 0;
    for (int i = 7; i < std::stoi(params[0]); i = i + 2) {
        if (params[i] == "startRound") {
            startRound = std::stoi(params[i + 1]);
        } else if (params[i] == "endRound") {
            endRound = std::stoi(params[i + 1]);
        } else if (params[i] == "timer") {
            timer = std::stoi(params[i + 1]);
        } else if (params[i] == "threadsNum") {
            threadsNum = std::stoi(params[i + 1]);
        } else if (params[i] == "keySize") {
            keySize = std::stoi(params[i + 1]);
        } else if (params[i] == "totalRoundNum") {
            totalRoundNum = std::stoi(params[i + 1]);
        } else
            assert(false);
    }

    std::string sboxModelingMode;
    if (params[4] == "w" or params[4] == "b") sboxModelingMode = "AS";
    else if (params[4] == "d") sboxModelingMode = "DC";
    else
        assert(false);

    if (params[5] == "evaluation" and (keySize == 0 or totalRoundNum == 0)) {
        std::cout << "the parameters 'keySize' and 'totalRoundNum' should be given when the target is 'evaluation' !" << std::endl;
        assert(false);
    }

    if (params[2] == "differential") {
        if (params[3] == "singleKey") {
            if (params[4] == "w") {
                DiffSWMILP sw(transformer.getProcedureHs(), params[5]);
                sw.setGurobiTimer(timer);
                sw.setGurobiThreads(threadsNum);
                sw.setStartRound(startRound);
                sw.setEndRound(endRound);
                if (params[5] == "cryptanalysis") {
                    sw.setSpeedUp1();
                    sw.setSpeedUp2();
                    sw.setILP();
                }
                sw.setKeySize(keySize);
                sw.setTotalRoundNum(totalRoundNum);
                sw.MGR();
            }
                // bit-wise or extended bit-wise
            else if (params[4] == "b" or params[4] == "d") {
                DiffSBMILP sb(transformer.getProcedureHs(), params[5], sboxModelingMode, std::stoi(params[6]));
                sb.setGurobiTimer(timer);
                sb.setGurobiThreads(threadsNum);
                sb.setStartRound(startRound);
                sb.setEndRound(endRound);
                if (params[5] == "cryptanalysis") {
                    sb.setSpeedUp1();
                    sb.setSpeedUp2();
                    // rectangle 的ILP和MILP进行evaluation时结果可能不一样,
                    // 因此为了和cryptanalysis的结果一致，必须用ILP的变量取值范围
                    // 实际上在转换为可满足行问题以后，已经不能称之为ILP或者MILP
                    // 正确的说法是，对于可满足性问题，其变量的取值会影响是否满足的结果
                    sb.setILP();
                }
                sb.setKeySize(keySize);
                sb.setTotalRoundNum(totalRoundNum);
                sb.MGR();
            } else {
                std::cout << "the fourth argument should be 'w', 'b' or 'd' !" << std::endl;
                assert(false);
            }
        } else if (params[3] == "relatedKey") {
            if (params[4] == "w") {
                DiffRWMILP rw(transformer.getProcedureHs(), params[5]);
                rw.setGurobiTimer(timer);
                rw.setGurobiThreads(threadsNum);
                rw.setStartRound(startRound);
                rw.setEndRound(endRound);
                if (params[5] == "cryptanalysis") {
                    //rw.setSpeedUp1();
                    //rw.setSpeedUp2();
                    //rw.setILP();
                }
                rw.setKeySize(keySize);
                rw.setTotalRoundNum(totalRoundNum);
                rw.MGR();
            }
            // bit-wise or extended bit-wise
            else if (params[4] == "b" or params[4] == "d") {
                DiffRBMILP rb(transformer.getProcedureHs(), params[5], sboxModelingMode, std::stoi(params[6]));
                rb.setGurobiTimer(timer);
                rb.setGurobiThreads(threadsNum);
                rb.setStartRound(startRound);
                rb.setEndRound(endRound);
                if (params[5] == "cryptanalysis") {
                    //rb.setSpeedUp1();
                    rb.setSpeedUp2();
                    rb.setILP();
                }
                rb.setKeySize(keySize);
                rb.setTotalRoundNum(totalRoundNum);
                rb.MGR();
            } else {
                std::cout << "the fourth argument should be 'w', 'b' or 'd' !" << std::endl;
                assert(false);
            }
        } else {
            std::cout << "the third argument should be 'singleKey' or 'relatedKey' !" << std::endl;
            assert(false);
        }
    } else if (params[2] == "linear") {
        if (params[3] == "singleKey") {
            if (params[4] == "w") {

            }
            // bit-wise or extended bit-wise
            else if (params[4] == "b" or params[4] == "d") {
                LinearSBMILP sb(transformer.getProcedureHs(), params[5], sboxModelingMode, std::stoi(params[6]));
                sb.setGurobiTimer(timer);
                sb.setGurobiThreads(threadsNum);
                sb.setStartRound(startRound);
                sb.setEndRound(endRound);
                if (params[5] == "cryptanalysis") {
                    sb.setSpeedUp1();
                    sb.setSpeedUp2();
                    // rectangle 的ILP和MILP进行evaluation时结果可能不一样,
                    // 因此为了和cryptanalysis的结果一致，必须用ILP的变量取值范围
                    // 实际上在转换为可满足行问题以后，已经不能称之为ILP或者MILP
                    // 正确的说法是，对于可满足性问题，其变量的取值会影响是否满足的结果
                    sb.setILP();
                }
                sb.setKeySize(keySize);
                sb.setTotalRoundNum(totalRoundNum);
                sb.MGR();
            } else {
                std::cout << "the fourth argument should be 'w', 'b' or 'd' !" << std::endl;
                assert(false);
            }
        } else if (params[3] == "relatedKey") {


        } else {
            std::cout << "the third argument should be 'singleKey' or 'relatedKey' !" << std::endl;
            assert(false);
        }
    }
}

void DivTrailsMGR(std::vector<std::string> params) {
    if (params.empty()) {
        std::cout << "Usage: ./EasyBC -div CIPHER [reductionMethod] [rounds] [activebits]" << std::endl;
        std::cout << "       activebits: integer (e.g. 60) for MSB-first legacy semantics," << std::endl;
        std::cout << "                   or R<k> / L<m> / L<m>R<k> for SIMON/Simeck right/left word." << std::endl;
        return;
    }

    std::string divCipherName = params[0];

    // 可选：第 2 个参数指定约简方法 (1..7)，缺省为 1 (greedy_sun)
    // 后续 rounds/activebits 依次后移到第 3、4 个参数（仅用于触发 MILP 建模）
    int reductionMethod = 1;
    if (params.size() >= 2) {
        try { reductionMethod = std::stoi(params[1]); }
        catch (...) { reductionMethod = 1; }
    }
    std::cout << "Reduction method: " << reductionMethod << std::endl;

    // Structured bench header — emitted regardless of whether MILP modeling
    // step runs, so a parser can attribute subsequent [BENCH] phase lines.
    {
        std::string _bench_rounds = (params.size() >= 3) ? params[2] : "-";
        std::string _bench_activebits = (params.size() >= 4) ? params[3] : "-";
        std::cerr << "[BENCH] config cipher=" << divCipherName
                  << " reduction=" << reductionMethod
                  << " rounds=" << _bench_rounds
                  << " activebits=" << _bench_activebits << std::endl;
    }

    // 查找对应的 .cl 文件名
    std::string RunCipherName = setup::cryptPrimitiveMap[divCipherName];
    std::string filePath = "../benchmarks/" + setup::cryptPrimitiveSetMap[divCipherName] + "/" + RunCipherName + ".cl";

    // 复用 EasyBC 的 .cl 文件解析流程
    yyin = fopen(filePath.c_str(), "r");
    if (!yyin) {
        std::cout << "Wrong Path : " << filePath << std::endl;
        assert(false);
    }
    yydebug = 0;
    yylineno = 1;
    Interpreter interpreter;
    if (!yyparse()) {
        std::cout << "Parsing complete" << std::endl;
    } else {
        std::cout << "Hint : wrong syntax at line " << yylineno << std::endl;
        assert(false);
    }

    // 遍历整棵 AST 的语义，把源码里的变量、函数、S-box/P-box、过程定义等信息，转成程序内部可用的数据结构。
    interpreter.generateCode(*programRoot);

    // 创建输出目录: data/division/{cipher}/
    std::string outputDir = "../data/division/" + divCipherName + "/";
    system(("mkdir -p " + outputDir).c_str());

    std::cout << "\n===== Start Performing Division Property Analysis(3 Steps) =====" << std::endl;
    std::cout << "\nStep 1: Generate Division Trails " << std::endl;
    std::cout << "\nStep 2: Generate and Reduce Inequalities" << std::endl;
    std::cout << "\nStep 3: MILP Modeling" << std::endl;
    std::cout << "CipherName: " << divCipherName << std::endl;

    bool hasSbox = false;
    for (auto& box : allBox) {
        if (box.first.substr(0, 4) == "sbox") {
            hasSbox = true;
            std::string sboxName = box.first;
            std::cout << "\nProcessing " << sboxName << " ..." << std::endl;

            SboxDivTrails sboxDivTrails(sboxName, box.second);
            sboxDivTrails.createDivisionTrails();

            std::string trailsFile = outputDir + sboxName + "_DivisionTrails.txt";
            sboxDivTrails.printDivisionTrails(trailsFile);

            DivTrailsModel model(divCipherName, sboxName,
                                 sboxDivTrails.getDivisionTrails(),
                                 sboxDivTrails.getSboxBitSize());
            model.generateInequalities();
            model.saveInequalities(outputDir);
            model.reduceInequalities(reductionMethod);
            model.saveReducedInequalities(outputDir);
        }
    }

    if (!hasSbox) {
        std::cout << "\nNo S-box detected; skipping inequality generation "
                     "(SIMON-like cipher)." << std::endl;
    }

    std::cout << "\n===== Division Trails and Inequalities Calculations Completed =====" << std::endl;

    if (params.size() >= 4) {
        int divRounds = std::stoi(params[2]);
        // activebits spec: plain integer (e.g. "60") keeps legacy MSB-first
        // semantics; "R<k>" / "L<m>R<k>" / "L<m>" target the right/left word
        // of a Feistel-style cipher (SIMON, Simeck). See Div2SetMILP.cpp.
        std::string divActivebitsSpec = params[3];

        // Run Transformer to get procedureHs for TAC traversal
        std::vector<ProcValuePtr> procs = interpreter.getProcs();
        Transformer transformer(procs);
        transformer.transformProcedures();
        std::vector<ProcedureHPtr> procedureHs = transformer.getProcedureHs();

        Div2SetMILP div2set(procedureHs, divRounds, divActivebitsSpec, divCipherName);

        // Parse optional parameters: timer and threads (从第 5 个参数开始成对解析)
        for (int i = 4; i < (int)params.size() - 1; i += 2) {
            if (params[i] == "timer") {
                div2set.setGurobiTimer(std::stoi(params[i + 1]));
            } else if (params[i] == "threads") {
                div2set.setGurobiThreads(std::stoi(params[i + 1]));
            }
        }

        div2set.MGR();
    }
}
