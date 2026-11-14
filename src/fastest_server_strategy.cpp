#include "FastestServerStrategy.h"

std::optional<std::size_t> FastestServerStrategy::chooseServer(const std::vector<Server>& servers) {
    if (servers.empty()) {
        return std::nullopt;
    }
    std::optional<std::size_t> fastestIndex;
    double fastestSpeed = 0.0;
    for (std::size_t i = 0; i < servers.size(); ++i) {
        const Server& server = servers[i];
        if (!server.canAcceptRequest()) {
            continue;
        }
        const double fastestCurrentServer = server.getProcessingSpeed();
        if (!fastestIndex || fastestCurrentServer > fastestSpeed || 
            (fastestCurrentServer == fastestSpeed && server.getId() < servers[*fastestIndex].getId())) {
            fastestSpeed = fastestCurrentServer;
            fastestIndex = i;
        }
    }
    return fastestIndex;
}

std::unique_ptr<RoutingStrategy> FastestServerStrategy::clone() const {
    return std::make_unique<FastestServerStrategy>(*this);
}

void FastestServerStrategy::print(std::ostream& os) const {
    os << "FastestServerStrategy";
}