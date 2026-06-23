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
};

struct BdptRunConfig {
    // Production defaults for EasyBC's 3-subset backend:
    //   - Paper: Algorithm 3's safe Key-XOR cross constraints
    //   - MinPin: enumerate reachable unit outputs efficiently
    //   - signLabeling=false: report NBB by default; M_L parity is not part of
    //     the normal Table-1-style distinguisher search.
    BdptCrossMode crossMode = BdptCrossMode::Paper;
    BdptUnitSearchMode unitSearchMode = BdptUnitSearchMode::MinPin;
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
