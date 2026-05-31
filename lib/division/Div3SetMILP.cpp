#include "division/Div3SetMILP.h"
#include <iostream>
#include <utility>

Div3SetMILP::Div3SetMILP(std::vector<ProcedureHPtr> procedureHs, int rounds,
                         const std::string& activebitsSpec, const std::string& cipherName)
        : procedureHs(std::move(procedureHs)), rounds(rounds),
          activebitsSpec(activebitsSpec), cipherName(cipherName) {}

void Div3SetMILP::MGR() {
    std::cout << "\n===== Step 3: 3-subset BDPT MILP Modeling =====" << std::endl;
    std::cout << "Cipher: " << this->cipherName
              << ", Rounds: " << this->rounds
              << ", Active bits: " << this->activebitsSpec << std::endl;
    std::cout << "Div3SetMILP: BDPT MILP modeling not yet implemented (Phase 2+).\n"
              << "  S-box K and L division-trail inequalities have been generated\n"
              << "  (see *_Reduce_Inequalities.txt and *_L_Reduce_Inequalities.txt).\n"
              << "  Remaining phases (K/L-chain walker, Key-XOR cross propagation,\n"
              << "  per-output-bit decision) are described in doc/three_subset_bdpt_plan.md."
              << std::endl;
}
