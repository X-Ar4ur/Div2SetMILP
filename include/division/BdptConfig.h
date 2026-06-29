#ifndef EASYBC_BDPTCONFIG_H
#define EASYBC_BDPTCONFIG_H

#include <string>
#include <vector>

struct BdptRunConfig {
    // Engineering runtime controls only. The 3-subset backend chooses its
    // modeling/solving strategy internally.
    int timerSeconds = 86400;
    int threads = 8;
};

bool parseBdptOptionalArgs(const std::vector<std::string>& args,
                           BdptRunConfig& config,
                           std::string& error);

#endif // EASYBC_BDPTCONFIG_H
