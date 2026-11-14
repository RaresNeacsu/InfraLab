#ifndef FASTEST_SERVER_STRATEGY_H
#define FASTEST_SERVER_STRATEGY_H

#include "RoutingStrategy.h"

class FastestServerStrategy : public RoutingStrategy {
public:
    std::optional<std::size_t> chooseServer(const std::vector<Server>& servers) override;
    std::unique_ptr<RoutingStrategy> clone() const override;
protected:
    void print(std::ostream& os) const override;
};

#endif // FASTEST_SERVER_STRATEGY_H