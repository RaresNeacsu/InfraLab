#ifndef ROUTING_STRATEGY_H
#define ROUTING_STRATEGY_H
#include <vector>
#include <iostream>
#include <optional>
#include <cstddef>
#include <memory>
#include "core/Server.h"


class RoutingStrategy {
    public:
        RoutingStrategy() = default;
        virtual ~RoutingStrategy() = default;
        std::optional<std::size_t> chooseServer(const std::vector<Server>& servers);
        friend std::ostream& operator<<(std::ostream& os, const RoutingStrategy& strategy);
        virtual std::unique_ptr<RoutingStrategy> clone() const = 0;
    protected:
        virtual std::optional<std::size_t> chooseServerImpl(const std::vector<Server>& servers) = 0;
        virtual void print(std::ostream& os) const = 0;
};

#endif // ROUTING_STRATEGY_H
