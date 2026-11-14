#include "LeastLoadedStrategy.h"

std::optional<std::size_t> LeastLoadedStrategy::chooseServer(const std::vector<Server>& servers) {
    if (servers.empty()) {
        return std::nullopt;
    }
    std::optional<std::size_t> leastLoadedIndex;
    std::size_t leastLoad = 0;
    for (std::size_t i = 0; i < servers.size(); ++i) {
        const Server& server = servers[i];
        if (!server.canAcceptRequest()) {
            continue;
        }
        const std::size_t currentLoad = static_cast<std::size_t>(server.getWaitingQueueSize()) + static_cast<std::size_t>(server.getActiveQueueSize());
        if (!leastLoadedIndex || currentLoad < leastLoad || (currentLoad == leastLoad && server.getId() < servers[*leastLoadedIndex].getId())) {
            leastLoad = currentLoad;
            leastLoadedIndex = i;
        }
    }
    return leastLoadedIndex;
}

std::unique_ptr<RoutingStrategy> LeastLoadedStrategy::clone() const {
    return std::make_unique<LeastLoadedStrategy>(*this);
}

void LeastLoadedStrategy::print(std::ostream& os) const {
    os << "LeastLoadedStrategy";
}
