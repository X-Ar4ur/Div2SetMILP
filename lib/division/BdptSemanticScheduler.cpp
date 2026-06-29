#include "division/BdptSemanticScheduler.h"

#include <algorithm>

namespace {

void appendScheduled(std::vector<BdptScheduledCrossLayer>& scheduled,
                     const BdptKeyXorLayer& layer,
                     int feRoundsBeforeCross) {
    BdptScheduledCrossLayer item;
    item.modelNumber = (int)scheduled.size() + 1;
    item.feRoundsBeforeCross = std::max(0, feRoundsBeforeCross);
    item.layer = layer;
    scheduled.push_back(item);
}

} // namespace

std::vector<BdptScheduledCrossLayer>
scheduleBdptCrossLayers(const std::vector<BdptKeyXorLayer>& layers,
                        int rounds) {
    std::vector<BdptScheduledCrossLayer> scheduled;
    if (layers.empty() || rounds <= 1) return scheduled;

    const bool roundInputKeyXor = layers.front().beforeRoundCore;
    if (roundInputKeyXor) {
        for (const BdptKeyXorLayer& layer : layers) {
            if (layer.round <= 1 || layer.round > rounds) continue;
            appendScheduled(scheduled, layer, layer.round - 1);
        }
    } else {
        const int n = (int)layers.size();
        for (int i = 0; i + 1 < n; ++i) {
            appendScheduled(scheduled, layers[i], layers[i].round);
        }
    }

    return scheduled;
}
