#ifndef EASYBC_BDPTSOLVERESULT_H
#define EASYBC_BDPTSOLVERESULT_H

#include <map>
#include <set>
#include <string>
#include <vector>

struct BdptSolveResult {
    std::set<int> reachable;
    bool complete = true;
    int solveCount = 0;
    double solverSeconds = 0.0;
    int lastStatus = 0;
    std::string reason;
    std::map<int, int> coordinateStatus;
    std::vector<int> iterationStatuses;

    void markIncomplete(int status, const std::string& why) {
        complete = false;
        lastStatus = status;
        reason = why;
    }
};

#endif // EASYBC_BDPTSOLVERESULT_H
