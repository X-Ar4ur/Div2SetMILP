#include "division/BdptConfig.h"

#include <cassert>
#include <string>
#include <vector>

int main() {
    {
        BdptRunConfig config;
        std::string error;
        assert(parseBdptOptionalArgs({}, config, error));
        assert(config.crossMode == BdptCrossMode::Exact);
        assert(config.unitSearchMode == BdptUnitSearchMode::MinPin);
        assert(config.signLabeling);
        assert(!config.reproduction);
        assert(config.timerSeconds == 86400);
        assert(config.threads == 8);
    }

    {
        BdptRunConfig config;
        std::string error;
        const std::vector<std::string> args = {
            "cross", "paper",
            "solver", "per-bit",
            "sign", "1",
            "repro", "1",
            "timer", "600",
            "threads", "4",
        };
        assert(parseBdptOptionalArgs(args, config, error));
        assert(config.crossMode == BdptCrossMode::Paper);
        assert(config.unitSearchMode == BdptUnitSearchMode::PerBit);
        assert(config.signLabeling);
        assert(config.reproduction);
        assert(config.timerSeconds == 600);
        assert(config.threads == 4);
        assert(toString(config.crossMode) == "paper");
        assert(toString(config.unitSearchMode) == "per-bit");
    }

    {
        BdptRunConfig config;
        std::string error;
        assert(!parseBdptOptionalArgs({"cross", "relaxed"}, config, error));
        assert(error.find("cross") != std::string::npos);
    }

    {
        BdptRunConfig config;
        std::string error;
        assert(!parseBdptOptionalArgs({"solver", "per-bit", "repro"}, config, error));
        assert(error.find("pairs") != std::string::npos);
    }

    {
        BdptRunConfig config;
        std::string error;
        assert(!parseBdptOptionalArgs({"unknown", "1"}, config, error));
        assert(error.find("unknown") != std::string::npos);
    }

    {
        BdptRunConfig config;
        std::string error;
        assert(!parseBdptOptionalArgs(
            {"repro", "1", "sign", "0"}, config, error));
        assert(error.find("sign 1") != std::string::npos);
    }

    return 0;
}
