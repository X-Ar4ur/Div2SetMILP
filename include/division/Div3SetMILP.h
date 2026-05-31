//
// 3-subset bit-based division property (BDPT) MILP Manager — Algorithm 3 + 4.
//
// Coexists with the 2-subset Div2SetMILP (CBDP). Selected via the `-div3`
// subcommand. The full pipeline (S-box K/L division-trail inequalities,
// Key-XOR cross propagation, per-output-bit decision) is built incrementally;
// see doc/three_subset_bdpt_plan.md. Phase 0 ships this skeleton only — MGR()
// prints a placeholder; Phases 2-4 fill in the model-set construction and the
// counting solver.
//

#ifndef EASYBC_DIV3SETMILP_H
#define EASYBC_DIV3SETMILP_H

#include <string>
#include <vector>
#include <utility>
#include "ProcedureH.h"

class Div3SetMILP {

private:
    std::vector<ProcedureHPtr> procedureHs;
    int rounds;
    std::string activebitsSpec;
    std::string cipherName;

    int gurobiTimer = 3600 * 24;
    int gurobiThreads = 8;

public:
    Div3SetMILP(std::vector<ProcedureHPtr> procedureHs, int rounds,
                const std::string& activebitsSpec, const std::string& cipherName);

    void setGurobiTimer(int timer) { this->gurobiTimer = timer; }
    void setGurobiThreads(int threads) { this->gurobiThreads = threads; }

    void MGR();
};

#endif //EASYBC_DIV3SETMILP_H
