#ifndef FASTEST_SERVER_STRATEGY_H
#define FASTEST_SERVER_STRATEGY_H

#include "routing/RoutingStrategy.h"

class FastestServerStrategy : public RoutingStrategy {
public:
    FastestServerStrategy();
    std::unique_ptr<RoutingStrategy> clone() const override;
protected:
    std::optional<std::size_t> chooseServerImpl(const std::vector<Server>& servers) override;
    void print(std::ostream& os) const override;
};

#endif // FASTEST_SERVER_STRATEGY_H
