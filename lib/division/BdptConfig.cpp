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

bool parseBoolean(const std::string& text, bool& value) {
    if (text == "0") {
        value = false;
        return true;
    }
    if (text == "1") {
        value = true;
        return true;
    }
    return false;
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

        if (key == "cross") {
            if (value == "paper") config.crossMode = BdptCrossMode::Paper;
            else if (value == "exact") config.crossMode = BdptCrossMode::Exact;
            else {
                error = "invalid cross mode '" + value + "' (expected paper or exact)";
                return false;
            }
        } else if (key == "solver") {
            if (value == "per-bit") config.unitSearchMode = BdptUnitSearchMode::PerBit;
            else if (value == "min-pin") config.unitSearchMode = BdptUnitSearchMode::MinPin;
            else {
                error = "invalid solver mode '" + value + "' (expected per-bit or min-pin)";
                return false;
            }
        } else if (key == "sign") {
            if (!parseBoolean(value, config.signLabeling)) {
                error = "invalid sign value '" + value + "' (expected 0 or 1)";
                return false;
            }
        } else if (key == "repro") {
            if (!parseBoolean(value, config.reproduction)) {
                error = "invalid repro value '" + value + "' (expected 0 or 1)";
                return false;
            }
        } else if (key == "timer") {
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

    if (config.reproduction && !config.signLabeling) {
        error = "repro 1 requires sign 1 for the paper parity check";
        return false;
    }

    error.clear();
    return true;
}

std::string toString(BdptCrossMode mode) {
    return mode == BdptCrossMode::Paper ? "paper" : "exact";
}

std::string toString(BdptUnitSearchMode mode) {
    return mode == BdptUnitSearchMode::PerBit ? "per-bit" : "min-pin";
}
