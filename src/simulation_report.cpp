#include "SimulationReport.h"
#include <stdexcept>
#include <unordered_set>

SimulationReport::SimulationReport(const std::vector<Request> &completedRequests, const std::vector<Request> &rejectedRequests, const std::vector<ServerStats> &serverReports)
    : completedRequests(completedRequests), rejectedRequests(rejectedRequests), serverReports(serverReports)
{
    std::unordered_set<int> requestIds;
    for (const auto &request : completedRequests)
    {
        if (request.getState() != Request::State::COMPLETED || !requestIds.insert(request.getId()).second)
        {
            throw std::invalid_argument("All requests in completedRequests must be in COMPLETED state.");
        }
    }
    for (const auto &request : rejectedRequests)
    {
        if (request.getState() != Request::State::REJECTED || !requestIds.insert(request.getId()).second)
        {
            throw std::invalid_argument("All requests in rejectedRequests must be in REJECTED state.");
        }
    }
    if (!serverReports.empty()) {
        std::unordered_set<int> serverIds;
        std::size_t remainingCompleted = completedRequests.size();
        std::size_t remainingRejected = rejectedRequests.size();
        for (const auto& stats : serverReports) {
            const auto completed = static_cast<std::size_t>(stats.getCompletedRequestsCount());
            const auto rejected = static_cast<std::size_t>(stats.getRejectedRequestsCount());
            if (!serverIds.insert(stats.getId()).second ||
                completed > remainingCompleted || rejected > remainingRejected) {
                throw std::invalid_argument("Server statistics do not match report results.");
            }
            remainingCompleted -= completed;
            remainingRejected -= rejected;
        }
        if (remainingCompleted != 0 || remainingRejected != 0) {
            throw std::invalid_argument("Server statistics do not cover every report result.");
        }
    }

};

double SimulationReport::averageWaitingTime() const
{
    if (completedRequests.empty())
    {
        throw std::runtime_error("No completed requests to calculate average waiting time.");
    }
    double totalWaitingTime = 0.0f;
    for (const auto &request : completedRequests)
    {
        totalWaitingTime += request.waitingTime() / completedRequests.size();
    }
    return totalWaitingTime;
}

double SimulationReport::averageResponseTime() const
{
    if (completedRequests.empty())
    {
        throw std::runtime_error("No completed requests to calculate average response time.");
    }
    double totalResponseTime = 0.0f;
    for (const auto &request : completedRequests)
    {
        totalResponseTime += request.responseTime() / completedRequests.size();
    }
    return totalResponseTime;
}

void SimulationReport::writeWaitingTimeAnalysis(std::ostream& os) const
{
    const ServerStats* selected = nullptr;
    for (const auto& stats : serverReports)
    {
        if (!stats.hasTimeStatistics()) continue;
        if (selected == nullptr || stats.getAverageWaitingTime() > selected->getAverageWaitingTime() ||
            (stats.getAverageWaitingTime() == selected->getAverageWaitingTime() && stats.getId() < selected->getId()))
        {
            selected = &stats;
        }
    }
    os << "Highest average waiting time (completed requests only): ";
    if (selected == nullptr)
    {
        os << "N/A - no server has timing statistics.\n";
    }
    else
    {
        os << "Server " << selected->getId() << ", "
           << selected->getAverageWaitingTime() << " s\n";
    }
}

std::ostream &operator<<(std::ostream &os, const SimulationReport &report)
{
    os << "SIMULATION REPORT\n=================\n\n";
    os << "Completed Requests: " << report.getCompletedRequestsCount() << "\n";
    for (const auto &request : report.completedRequests)
    {
        os << "  " << request << "\n";
    }
    os << "\n";
    os << "Rejected Requests: " << report.getRejectedRequestsCount() << "\n";
    for (const auto &request : report.rejectedRequests)
    {
        os << "  " << request << "\n";
    }
    os << "\n";
    if (report.getCompletedRequestsCount() > 0)
    {
        os << "Average Waiting Time: " << report.averageWaitingTime() << " s\n";
        os << "Average Response Time: " << report.averageResponseTime() << " s\n";
    }
    else
    {
        os << "No completed requests to calculate average times.\n";
    }
    if (!report.serverReports.empty())
    {
        os << "\nServer Statistics:\n";
        for (const auto& stats : report.serverReports)
        {
            os << "  " << stats << '\n';
        }
        os << "Rejected per server: global rejections attributed to the round-robin starting server\n"
              "after every server was found full; not a measure of an individual server bottleneck.\n";
        report.writeWaitingTimeAnalysis(os);
    }
    return os;
}
