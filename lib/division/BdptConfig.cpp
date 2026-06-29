#include "division/BdptConfig.h"

#include <exception>

namespace {

bool parsePositiveInt(const std::string& text, int& value) {
    try {
        size_t consumed = 0;
        int parsed = std::stoi(text, &consumed);
        if (consumed != text.size() || parsed <= 0) return false;
        value = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace

bool parseBdptOptionalArgs(const std::vector<std::string>& args,
                           BdptRunConfig& config,
                           std::string& error) {
    if (args.size() % 2 != 0) {
        error = "-div3 optional arguments must be key/value pairs";
        return false;
    }

    for (size_t i = 0; i < args.size(); i += 2) {
        const std::string& key = args[i];
        const std::string& value = args[i + 1];

        if (key == "timer") {
            if (!parsePositiveInt(value, config.timerSeconds)) {
                error = "invalid timer value '" + value + "' (expected a positive integer)";
                return false;
            }
        } else if (key == "threads") {
            if (!parsePositiveInt(value, config.threads)) {
                error = "invalid threads value '" + value + "' (expected a positive integer)";
                return false;
            }
        } else {
            error = "unknown -div3 option '" + key + "'";
            return false;
        }
    }

    error.clear();
    return true;
}
