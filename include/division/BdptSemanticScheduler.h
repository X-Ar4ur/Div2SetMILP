#ifndef EASYBC_BDPTSEMANTICSCHEDULER_H
#define EASYBC_BDPTSEMANTICSCHEDULER_H

#include <vector>

#include "BdptKeyXor.h"

struct BdptScheduledCrossLayer {
    int modelNumber = -1;
    int feRoundsBeforeCross = 0;
    BdptKeyXorLayer layer;
};

std::vector<BdptScheduledCrossLayer>
scheduleBdptCrossLayers(const std::vector<BdptKeyXorLayer>& layers,
                        int rounds);

#endif // EASYBC_BDPTSEMANTICSCHEDULER_H
