#ifndef LEAST_LOADED_STRATEGY_H
#define LEAST_LOADED_STRATEGY_H
#include "RoutingStrategy.h"

class LeastLoadedStrategy : public RoutingStrategy {
    public:
        LeastLoadedStrategy() = default;
        ~LeastLoadedStrategy() override = default;
        std::optional<std::size_t> chooseServer(const std::vector<Server>& servers) override;
        std::unique_ptr<RoutingStrategy> clone() const override;

    protected:
        void print(std::ostream& os) const override;
};


#endif // LEAST_LOADED_STRATEGY_H