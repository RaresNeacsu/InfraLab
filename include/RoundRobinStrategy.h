#ifndef ROUND_ROBIN_STRATEGY_H
#define ROUND_ROBIN_STRATEGY_H

#include "RoutingStrategy.h"

class RoundRobinStrategy : public RoutingStrategy {
    public:
        RoundRobinStrategy() = default;
        ~RoundRobinStrategy() override = default;
        std::optional<std::size_t> chooseServer(const std::vector<Server>& servers) override;
        std::unique_ptr<RoutingStrategy> clone() const override;

    protected:
        void print(std::ostream& os) const override;

    private:
        std::size_t currentIndex = 0;
};

#endif // ROUND_ROBIN_STRATEGY_H