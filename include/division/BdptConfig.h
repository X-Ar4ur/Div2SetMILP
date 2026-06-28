#ifndef EASYBC_BDPTCONFIG_H
#define EASYBC_BDPTCONFIG_H

#include <string>
#include <vector>

enum class BdptCrossMode {
    Paper,
    Exact,
};

enum class BdptUnitSearchMode {
    PerBit,
    MinPin,
    Hybrid,
};

struct BdptRunConfig {
    // Production defaults for EasyBC's 3-subset backend:
    //   - Exact: Rule 1 Key-XOR cross propagation (L_t with one touched zero bit
    //     raised into K_t*) as the framework-level BDPT semantics.
    //   - Hybrid: use the paper/reference minimize-and-pin loop as the default
    //     production strategy; per-coordinate feasibility remains internal and
    //     is not used as a large-candidate fallback.
    //   - signLabeling=false: report NBB by default; M_L parity is not part of
    //     the normal Table-1-style distinguisher search.
    BdptCrossMode crossMode = BdptCrossMode::Exact;
    BdptUnitSearchMode unitSearchMode = BdptUnitSearchMode::Hybrid;
    bool signLabeling = false;
    bool reproduction = false;
    int timerSeconds = 86400;
    int threads = 8;
};

bool parseBdptOptionalArgs(const std::vector<std::string>& args,
                           BdptRunConfig& config,
                           std::string& error);

std::string toString(BdptCrossMode mode);
std::string toString(BdptUnitSearchMode mode);

#endif // EASYBC_BDPTCONFIG_H
