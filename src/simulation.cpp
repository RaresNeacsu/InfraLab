#include "Simulation.h"
#include "Server.h"
#include "SimulationReport.h"
#include "ServerStats.h"
#include <stdexcept>
#include <cmath>
#include <limits>
#include <unordered_set>

Simulation::Simulation(const std::vector<Server> &servers, const std::vector<Request> &incomingRequests, double maximumTime)
    : servers(servers), incomingRequests(incomingRequests), maximumTime(maximumTime)
{
    if (servers.empty())
    {
        throw std::invalid_argument("Servers vector cannot be empty");
    }
    std::unordered_set<int> serverIds;
    for (const auto& server : servers) {
        if (!serverIds.insert(server.getId()).second) {
            throw std::invalid_argument("Server IDs must be unique.");
        }
    }
    std::unordered_set<int> requestIds;
    for (std::size_t i = 0; i < incomingRequests.size(); ++i) {
        const auto& request = incomingRequests[i];
        if (request.getState() != Request::State::PENDING ||
            !requestIds.insert(request.getId()).second) {
            throw std::invalid_argument("Incoming requests must be pending and have unique IDs.");
        }
        if (i > 0 && request.getArrivalTime() < incomingRequests[i - 1].getArrivalTime()) {
            throw std::invalid_argument("Incoming requests must be ordered by arrival time.");
        }
    }
    if (maximumTime <= 0.0f || std::isnan(maximumTime) || std::isinf(maximumTime))
    {
        throw std::invalid_argument("Maximum time must be a valid positive double");
    }
    
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
        std::size_t selectedServer = nextServerIndex;
        for (std::size_t attempts = 0; attempts < servers.size(); ++attempts)
        {
            const std::size_t candidate = (nextServerIndex + attempts) % servers.size();
            if (servers[candidate].canAcceptRequest())
            {
                selectedServer = candidate;
                break;
            }
        }
        // If every server is full, the starting server records one rejection.
        servers[selectedServer].submitRequest(incomingRequests[nextArrivalIndex], currentTime);
        nextServerIndex = (selectedServer + 1) % servers.size();
        nextArrivalIndex++;
    }
    return StepResult::EventProcessed;

}

SimulationReport Simulation::collectReport() const
{
    std::vector<Request> completedRequests;
    std::vector<Request> rejectedRequests;
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
    os << "Next Server Index: " << simulation.nextServerIndex << "\n";
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
