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
    BdptCrossMode crossMode = BdptCrossMode::Exact;
    BdptUnitSearchMode unitSearchMode = BdptUnitSearchMode::MinPin;
    bool signLabeling = true;
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
