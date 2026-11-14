#include "RoundRobinStrategy.h"

std::optional<std::size_t> RoundRobinStrategy::chooseServer(const std::vector<Server>& servers) {
    if (servers.empty()) {
        return std::nullopt;
    }
    for (std::size_t i = 0; i < servers.size(); ++i) {
        std::size_t index = (currentIndex + i) % servers.size();
        if (servers[index].canAcceptRequest()) {
            currentIndex = (index + 1) % servers.size();
            return index;
        }
    }
    return std::nullopt;
}

std::unique_ptr<RoutingStrategy> RoundRobinStrategy::clone() const {
    return std::make_unique<RoundRobinStrategy>(*this);
}

void RoundRobinStrategy::print(std::ostream& os) const {
    os << "RoundRobinStrategy(currentIndex=" << currentIndex << ")";
}