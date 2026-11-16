#ifndef ROUND_ROBIN_STRATEGY_H
#define ROUND_ROBIN_STRATEGY_H

#include "RoutingStrategy.h"

class RoundRobinStrategy : public RoutingStrategy {
    public:
        RoundRobinStrategy();
        ~RoundRobinStrategy() override = default;
        std::unique_ptr<RoutingStrategy> clone() const override;

    protected:
        std::optional<std::size_t> chooseServerImpl(const std::vector<Server>& servers) override;
        void print(std::ostream& os) const override;

    private:
        std::size_t currentIndex = 0;
};

#endif // ROUND_ROBIN_STRATEGY_H
