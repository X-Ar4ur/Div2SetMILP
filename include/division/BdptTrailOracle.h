#ifndef EASYBC_BDPTTRAILORACLE_H
#define EASYBC_BDPTTRAILORACLE_H

#include <map>
#include <set>
#include <string>

enum class BdptTrailKind {
    K,
    L,
};

class BdptTrailOracle {
private:
    int inSize = 0;
    int outSize = 0;
    std::set<std::pair<int, int>> kTrails;
    std::set<std::pair<int, int>> lTrails;
    std::map<int, int> kOutputs;
    std::map<int, int> lOutputs;

    static bool loadOne(const std::string& filename,
                        int inputSize,
                        int outputSize,
                        std::set<std::pair<int, int>>& trails,
                        std::map<int, int>& outputs,
                        std::string& error);

public:
    bool load(int inputSize,
              int outputSize,
              const std::string& kTrailFile,
              const std::string& lTrailFile,
              std::string& error);

    bool allows(BdptTrailKind kind, int inputMask, int outputMask) const;
    int possibleOutputs(BdptTrailKind kind, int inputMask) const;

    int inputSize() const { return inSize; }
    int outputSize() const { return outSize; }
    bool empty() const { return kTrails.empty() && lTrails.empty(); }
};

#endif // EASYBC_BDPTTRAILORACLE_H
