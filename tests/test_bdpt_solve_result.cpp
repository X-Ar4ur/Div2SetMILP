#include "division/BdptSolveResult.h"

#include <cassert>
#include <string>

int main() {
    BdptSolveResult result;
    assert(result.complete);
    assert(result.solveCount == 0);
    assert(result.solverSeconds == 0.0);
    assert(result.reachable.empty());
    assert(result.reason.empty());

    result.reachable.insert(7);
    result.coordinateStatus[7] = 2;
    result.iterationStatuses.push_back(15);
    result.solveCount = 3;
    result.solverSeconds = 1.25;
    result.markIncomplete(9, "time-limit");

    assert(!result.complete);
    assert(result.lastStatus == 9);
    assert(result.reason == "time-limit");
    assert(result.reachable.count(7) == 1);
    assert(result.coordinateStatus.at(7) == 2);
    assert(result.iterationStatuses.size() == 1);
    assert(result.iterationStatuses.front() == 15);

    return 0;
}
