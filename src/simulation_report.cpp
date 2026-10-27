#include "SimulationReport.h"
#include <stdexcept>

SimulationReport::SimulationReport(const std::vector<Request>& completedRequests, const std::vector<Request>& rejectedRequests) :
 completedRequests(completedRequests), rejectedRequests(rejectedRequests)
{
    for (const auto& request : completedRequests)
    {
        if(request.getState() != Request::State::COMPLETED)
        {
            throw std::invalid_argument("All requests in completedRequests must be in COMPLETED state.");
        }
    }
    for (const auto& request : rejectedRequests)
    {
        if(request.getState() != Request::State::REJECTED)
        {
            throw std::invalid_argument("All requests in rejectedRequests must be in REJECTED state.");
        }
    }
};

float SimulationReport::averageWaitingTime() const
{
    if(completedRequests.empty())
    {
        throw std::runtime_error("No completed requests to calculate average waiting time.");
    }
    float totalWaitingTime = 0.0f;
    for(const auto& request : completedRequests)
    {
        totalWaitingTime += request.waitingTime();
    }
    return totalWaitingTime / completedRequests.size();
}

float SimulationReport::averageResponseTime() const
{
    if(completedRequests.empty())
    {
        throw std::runtime_error("No completed requests to calculate average response time."); 
    }
    float totalResponseTime = 0.0f;
    for(const auto& request : completedRequests)
    {
        totalResponseTime += request.responseTime();
    }
    return totalResponseTime / completedRequests.size();
}

std::ostream& operator<<(std::ostream& os, const SimulationReport& report)
{
    os << "SIMULATION REPORT\n=================\n\n";
    os << "Completed Requests: " << report.getCompletedRequestsCount() << "\n";
    for(const auto& request : report.completedRequests)
    {
        os << "  " << request << "\n";
    }
    os << "\n";
    os << "Rejected Requests: " << report.getRejectedRequestsCount() << "\n";
    for(const auto& request : report.rejectedRequests)
    {
        os << "  " << request << "\n";
    }
    os << "\n";
    if(report.getCompletedRequestsCount() > 0)
    {
        os << "Average Waiting Time: " << report.averageWaitingTime() << " s\n";
        os << "Average Response Time: " << report.averageResponseTime() << " s\n";
    }
    else
    {
        os << "No completed requests to calculate average times.\n";
    }
    return os;
}