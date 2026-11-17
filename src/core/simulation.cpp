#include "errors/ConfigurationError.h"
#include "core/Simulation.h"
#include "core/Server.h"
#include "reports/SimulationReport.h"
#include "reports/ServerStats.h"
#include <stdexcept>
#include <cmath>
#include <limits>
#include <utility>
#include <unordered_set>

Simulation::Simulation(const std::vector<Server> &servers, const std::vector<Request> &incomingRequests, double maximumTime, std::unique_ptr<RoutingStrategy> strategy)
    : servers(servers), incomingRequests(incomingRequests), maximumTime(maximumTime), routingStrategy(std::move(strategy))
{
    if (servers.empty())
    {
        throw ConfigurationError("Servers vector cannot be empty");
    }
    if (!this->routingStrategy)
    {
        throw ConfigurationError("Routing strategy cannot be null");
    }
    std::unordered_set<int> serverIds;
    for (const auto& server : servers) {
        if (!serverIds.insert(server.getId()).second) {
            throw ConfigurationError("Server IDs must be unique.");
        }
    }
    std::unordered_set<int> requestIds;
    for (std::size_t i = 0; i < incomingRequests.size(); ++i) {
        const auto& request = incomingRequests[i];
        if (request.getState() != Request::State::PENDING ||
            !requestIds.insert(request.getId()).second) {
            throw ConfigurationError("Incoming requests must be pending and have unique IDs.");
        }
        if (i > 0 && request.getArrivalTime() < incomingRequests[i - 1].getArrivalTime()) {
            throw ConfigurationError("Incoming requests must be ordered by arrival time.");
        }
    }
    if (maximumTime <= 0.0f || std::isnan(maximumTime) || std::isinf(maximumTime))
    {
        throw ConfigurationError("Maximum time must be a valid positive double");
    }

}

Simulation::Simulation(const Simulation& other)
    : servers(other.servers), incomingRequests(other.incomingRequests),
      globallyRejectedRequests(other.globallyRejectedRequests), nextArrivalIndex(other.nextArrivalIndex),
      currentTime(other.currentTime), maximumTime(other.maximumTime),
      routingStrategy(other.routingStrategy->clone())
{
}

void Simulation::swap(Simulation& other) noexcept
{
    using std::swap;
    swap(servers, other.servers);
    swap(incomingRequests, other.incomingRequests);
    swap(globallyRejectedRequests, other.globallyRejectedRequests);
    swap(nextArrivalIndex, other.nextArrivalIndex);
    swap(currentTime, other.currentTime);
    swap(maximumTime, other.maximumTime);
    swap(routingStrategy, other.routingStrategy);
}

Simulation& Simulation::operator=(const Simulation& other)
{
    Simulation copy(other);
    swap(copy);
    return *this;
}

double Simulation::nextEventTime() const
{
    double earliestEventTime = std::numeric_limits<double>::infinity();
    if (nextArrivalIndex < incomingRequests.size())
    {
        earliestEventTime = incomingRequests[nextArrivalIndex].getArrivalTime();
    }
    for (const auto &server : servers)
    {
        double serverNextCompletionTime = server.nextCompletionTime();
        if (serverNextCompletionTime < earliestEventTime)
        {
            earliestEventTime = serverNextCompletionTime;
        }
    }
    return earliestEventTime;
}

Simulation::StepResult Simulation::step()
{
    double nextTime = nextEventTime();
    if (nextTime == std::numeric_limits<double>::infinity())
    {
        return StepResult::Completed;
    }
    if (nextTime > maximumTime)
    {
        currentTime = maximumTime;
        return StepResult::TimeLimitReached;
    }
    currentTime = nextTime;
    for(auto& server : servers)
    {
        server.completeActiveRequest(currentTime);
    }
    for (auto& server : servers)
    {
        server.waitingQueueToActive(currentTime);
    }
    while (nextArrivalIndex < incomingRequests.size() && incomingRequests[nextArrivalIndex].getArrivalTime() <= currentTime)
    {
        const auto selectedServer = routingStrategy->chooseServer(servers);
        if (selectedServer)
        {
            servers[*selectedServer].submitRequest(incomingRequests[nextArrivalIndex], currentTime);
        }
        else
        {
            Request rejected = incomingRequests[nextArrivalIndex];
            rejected.reject();
            globallyRejectedRequests.push_back(rejected);
        }
        nextArrivalIndex++;
    }
    return StepResult::EventProcessed;

}

SimulationReport Simulation::collectReport() const
{
    std::vector<Request> completedRequests;
    std::vector<Request> rejectedRequests = globallyRejectedRequests;
    std::vector<ServerStats> serverReports;
    for (const auto& server : servers)
    {
        std::vector<Request> completedLocalRequests;
        std::vector<Request> rejectedLocalRequests;
        server.collectCompletedRequests(completedLocalRequests);
        server.collectRejectedRequests(rejectedLocalRequests);
        rejectedRequests.insert(rejectedRequests.end(), rejectedLocalRequests.begin(), rejectedLocalRequests.end());
        completedRequests.insert(completedRequests.end(), completedLocalRequests.begin(), completedLocalRequests.end());
        int completedCount = 0;
        int rejectedCount = 0;
        int totalCount = 0;
        int maxQueueSize = 0;
        server.collectServerStatistics(completedCount, rejectedCount, totalCount, maxQueueSize);
        serverReports.emplace_back(server.getId(), completedCount, rejectedCount, totalCount, maxQueueSize);
        serverReports.back().calculateTimeStatistics(completedLocalRequests);
    }
    return SimulationReport(completedRequests, rejectedRequests, serverReports);
}

std::ostream& operator<<(std::ostream& os, const Simulation& simulation)
{
    os << "Simulation State:\n";
    os << "Current Time: " << simulation.currentTime << "\n";
    os << "Maximum Time: " << simulation.maximumTime << "\n";
    os << "Next Arrival Index: " << simulation.nextArrivalIndex << "\n";
    os << "Routing Strategy: " << *simulation.routingStrategy << "\n";
    os << "Incoming Requests:\n";
    for (const auto& request : simulation.incomingRequests)
    {
        os << "  " << request << "\n";
    }
    os << "\nServers:\n";
    for (const auto& server : simulation.servers)
    {
        os << server << "\n";
    }
    return os;
}
