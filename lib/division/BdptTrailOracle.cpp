#include "division/BdptTrailOracle.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <vector>

namespace {

int bitsToMask(const std::vector<int>& bits, int begin, int count) {
    int mask = 0;
    for (int i = 0; i < count; ++i) {
        if (bits[begin + i] != 0) mask |= (1 << i);
    }
    return mask;
}

bool parseTrailLine(const std::string& line, std::vector<int>& bits) {
    bits.clear();
    if (line.find('[') == std::string::npos) return false;
    for (char c : line) {
        if (c == '0' || c == '1') {
            bits.push_back(c - '0');
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return !bits.empty();
}

} // namespace

bool BdptTrailOracle::loadOne(const std::string& filename,
                              int inputSize,
                              int outputSize,
                              std::set<std::pair<int, int>>& trails,
                              std::map<int, int>& outputs,
                              std::string& error) {
    std::ifstream file(filename);
    if (!file) {
        error = "cannot open trail file: " + filename;
        return false;
    }

    trails.clear();
    outputs.clear();
    std::string line;
    int lineNo = 0;
    while (std::getline(file, line)) {
        ++lineNo;
        std::vector<int> bits;
        if (!parseTrailLine(line, bits)) continue;
        if ((int)bits.size() != inputSize + outputSize) {
            std::ostringstream oss;
            oss << "invalid trail width in " << filename << ":" << lineNo;
            error = oss.str();
            return false;
        }
        const int inMask = bitsToMask(bits, 0, inputSize);
        const int outMask = bitsToMask(bits, inputSize, outputSize);
        trails.insert(std::make_pair(inMask, outMask));
        outputs[inMask] |= outMask;
    }

    if (trails.empty()) {
        error = "no trails loaded from: " + filename;
        return false;
    }
    return true;
}

bool BdptTrailOracle::load(int inputSize,
                           int outputSize,
                           const std::string& kTrailFile,
                           const std::string& lTrailFile,
                           std::string& error) {
    if (inputSize <= 0 || outputSize <= 0 ||
        inputSize >= (int)(8 * sizeof(int)) ||
        outputSize >= (int)(8 * sizeof(int))) {
        error = "invalid S-box size for BDPT trail oracle";
        return false;
    }
    this->inSize = inputSize;
    this->outSize = outputSize;
    if (!loadOne(kTrailFile, inputSize, outputSize, kTrails, kOutputs, error)) {
        return false;
    }
    if (!loadOne(lTrailFile, inputSize, outputSize, lTrails, lOutputs, error)) {
        return false;
    }
    error.clear();
    return true;
}

bool BdptTrailOracle::allows(BdptTrailKind kind, int inputMask, int outputMask) const {
    const auto& trails = (kind == BdptTrailKind::L) ? lTrails : kTrails;
    return trails.count(std::make_pair(inputMask, outputMask)) != 0;
}

int BdptTrailOracle::possibleOutputs(BdptTrailKind kind, int inputMask) const {
    const auto& outputs = (kind == BdptTrailKind::L) ? lOutputs : kOutputs;
    auto it = outputs.find(inputMask);
    return it == outputs.end() ? 0 : it->second;
}
