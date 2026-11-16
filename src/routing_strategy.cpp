#include "RoutingStrategy.h"

std::optional<std::size_t> RoutingStrategy::chooseServer(const std::vector<Server>& servers)
{
    return chooseServerImpl(servers);
}

std::ostream& operator<<(std::ostream& os, const RoutingStrategy& strategy) {
    strategy.print(os);
    return os;
}
