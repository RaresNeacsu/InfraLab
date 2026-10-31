#include "Simulation.h"
#include "Server.h"
#include "SimulationReport.h"
#include "ServerStats.h"
#include <stdexcept>
#include <cmath>
#include <limits>

Simulation::Simulation(const std::vector<Server> &servers, const std::vector<Request> &incomingRequests, float maximumTime)
    : servers(servers), incomingRequests(incomingRequests), maximumTime(maximumTime)
{
    if (servers.empty())
    {
        throw std::invalid_argument("Servers vector cannot be empty");
    }
    if (!(incomingRequests.size() == 0 || incomingRequests.size() == 1))
    {
        for (std::size_t i = 1; i < incomingRequests.size(); i++)
        {
            if (incomingRequests[i].getArrivalTime() < incomingRequests[i - 1].getArrivalTime())
            {
                throw std::invalid_argument("Incoming requests must be ordered by traffic arrival time.");
            }
            if(incomingRequests[i].getState() != Request::State::PENDING)
            {
                throw std::invalid_argument("All incoming requests must be in PENDING state.");
            }
        }
        if (incomingRequests[0].getState() != Request::State::PENDING)
        {
            throw std::invalid_argument("All incoming requests must be in PENDING state.");
        }
    }

    if(incomingRequests.size()==1 && incomingRequests[0].getState() != Request::State::PENDING)
    {
        throw std::invalid_argument("All incoming requests must be in PENDING state.");
    }
    if (maximumTime <= 0.0f || std::isnan(maximumTime) || std::isinf(maximumTime))
    {
        throw std::invalid_argument("Maximum time must be a valid positive float");
    }
    
}

float Simulation::nextEventTime() const
{
    float earliestEventTime = std::numeric_limits<float>::infinity();
    if (nextArrivalIndex < incomingRequests.size())
    {
        earliestEventTime = incomingRequests[nextArrivalIndex].getArrivalTime();
    }
    for (const auto &server : servers)
    {
        float serverNextCompletionTime = server.nextCompletionTime();
        if (serverNextCompletionTime < earliestEventTime)
        {
            earliestEventTime = serverNextCompletionTime;
        }
    }
    return earliestEventTime;
}

Simulation::StepResult Simulation::step()
{
    float nextTime = nextEventTime();
    if (nextTime == std::numeric_limits<float>::infinity())
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
        // Existing collection methods drain queues; drain a local snapshot only.
        Server snapshot(server);
        snapshot.collectCompletedRequests(completedRequests);
        snapshot.collectRejectedRequests(rejectedRequests);
        int completedCount = 0;
        int rejectedCount = 0;
        int totalCount = 0;
        int maxQueueSize = 0;
        server.collectServerStatistics(completedCount, rejectedCount, totalCount, maxQueueSize);
        serverReports.emplace_back(server.getId(), completedCount, rejectedCount, totalCount, maxQueueSize);
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
