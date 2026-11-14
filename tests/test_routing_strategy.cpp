#undef NDEBUG
#include <cassert>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "FastestServerStrategy.h"
#include "LeastLoadedStrategy.h"
#include "RoundRobinStrategy.h"

void testRoutingStrategy()
{
    std::vector<Server> servers{
        Server(30, 3.0, 1, 1), Server(10, 3.0, 1, 1), Server(20, 1.0, 1, 1)
    };
    RoundRobinStrategy roundRobin;
    LeastLoadedStrategy leastLoaded;
    FastestServerStrategy fastest;
    std::vector<RoutingStrategy*> strategies{&roundRobin, &leastLoaded, &fastest};
    for (auto* strategy : strategies) {
        const auto result = strategy->chooseServer({});
        assert(!result);
    }

    auto result = roundRobin.chooseServer(servers);
    assert(result && *result == 0);
    auto copy = roundRobin.clone();
    result = roundRobin.chooseServer(servers);
    assert(result && *result == 1);
    result = copy->chooseServer(servers);
    assert(result && *result == 1);
    result = roundRobin.chooseServer(servers);
    assert(result && *result == 2);
    result = roundRobin.chooseServer(servers);
    assert(result && *result == 0);

    result = leastLoaded.chooseServer(servers);
    assert(result && *result == 1);
    result = fastest.chooseServer(servers);
    assert(result && *result == 1);

    const bool firstAccepted = servers[1].submitRequest(Request(1, 0.0, 1.0), 0.0);
    assert(firstAccepted);
    result = leastLoaded.chooseServer(servers);
    assert(result && *result == 2);
    const bool secondAccepted = servers[1].submitRequest(Request(2, 0.0, 1.0), 0.0);
    assert(secondAccepted);
    result = fastest.chooseServer(servers);
    assert(result && *result == 0);
    result = roundRobin.chooseServer(servers);
    assert(result && *result == 2);

    for (std::size_t index : {std::size_t{0}, std::size_t{2}}) {
        const bool activeAccepted = servers[index].submitRequest(Request(3, 0.0, 1.0), 0.0);
        const bool queuedAccepted = servers[index].submitRequest(Request(4, 0.0, 1.0), 0.0);
        assert(activeAccepted && queuedAccepted);
    }
    for (auto* strategy : strategies) {
        const auto fullResult = strategy->chooseServer(servers);
        assert(!fullResult);
        const auto cloned = strategy->clone();
        std::ostringstream originalOutput;
        std::ostringstream clonedOutput;
        originalOutput << *strategy;
        clonedOutput << *cloned;
        assert(!originalOutput.str().empty());
        assert(originalOutput.str() == clonedOutput.str());
    }
}
