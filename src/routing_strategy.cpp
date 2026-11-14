#include "RoutingStrategy.h"

std::ostream& operator<<(std::ostream& os, const RoutingStrategy& strategy) {
    strategy.print(os);
    return os;
}