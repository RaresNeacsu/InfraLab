#ifndef LEAST_LOADED_STRATEGY_H
#define LEAST_LOADED_STRATEGY_H
#include "routing/RoutingStrategy.h"

class LeastLoadedStrategy : public RoutingStrategy {
    public:
        LeastLoadedStrategy();
        ~LeastLoadedStrategy() override = default;
        std::unique_ptr<RoutingStrategy> clone() const override;

    protected:
        std::optional<std::size_t> chooseServerImpl(const std::vector<Server>& servers) override;
        void print(std::ostream& os) const override;
};


#endif // LEAST_LOADED_STRATEGY_H
