#include "ServerStats.h"
#include "Request.h"
#include <stdexcept>
#include <vector>

ServerStats::ServerStats(int id, int completedRequestsCount, int rejectedRequestsCount, int totalRequestsCount, int maxWaitingQueueSize)
    : id(id), completedRequestsCount(completedRequestsCount), rejectedRequestsCount(rejectedRequestsCount), 
    totalRequestsCount(totalRequestsCount), maxWaitingQueueSize(maxWaitingQueueSize)
{
    if (id <= 0) {
        throw std::invalid_argument("Server statistics ID must be positive.");
    }
    if (completedRequestsCount < 0 || rejectedRequestsCount < 0 ||
        totalRequestsCount < 0 || maxWaitingQueueSize < 0) {
        throw std::invalid_argument("Server statistics counters must be non-negative.");
    }
    // Subtract only after checking the bounds, avoiding signed overflow.
    if (completedRequestsCount > totalRequestsCount ||
        rejectedRequestsCount > totalRequestsCount - completedRequestsCount) {
        throw std::invalid_argument("Completed and rejected counts cannot exceed total requests.");
    }
    if (maxWaitingQueueSize > totalRequestsCount - rejectedRequestsCount) {
        throw std::invalid_argument("Queue peak cannot exceed the number of accepted requests.");
    }
}

double ServerStats::getRejectionRate() const
{
    if (totalRequestsCount == 0) {
        return 0.0;
    }
    return static_cast<double>(rejectedRequestsCount) / totalRequestsCount;
}

double ServerStats::getCompletionRate() const
{
    if (totalRequestsCount == 0) {
        return 0.0;
    }
    return static_cast<double>(completedRequestsCount) / totalRequestsCount;
}

double ServerStats::getUnprocessedRate() const
{
    if (totalRequestsCount == 0) {
        return 0.0;
    }
    const int unprocessedCount = totalRequestsCount - completedRequestsCount - rejectedRequestsCount;
    return static_cast<double>(unprocessedCount) / totalRequestsCount;
}

void ServerStats::calculateTimeStatistics(const std::vector<Request>& completedRequests)
{
    if (completedRequests.size() != static_cast<std::size_t>(completedRequestsCount)) {
        throw std::invalid_argument("Timing sample count must match completed requests count.");
    }
    double sumWaitingTime = 0.0;
    double sumResponseTime = 0.0;
    double waitingMaximum = 0.0;
    double responseMaximum = 0.0;
    for (const auto& request : completedRequests)
    {
        const double waitingTime = request.waitingTime();
        const double responseTime = request.responseTime();
        sumWaitingTime += waitingTime / completedRequests.size();
        sumResponseTime += responseTime / completedRequests.size();
        if (waitingTime > waitingMaximum) waitingMaximum = waitingTime;
        if (responseTime > responseMaximum) responseMaximum = responseTime;
    }
    maxWaitingTime = waitingMaximum;
    maxResponseTime = responseMaximum;
    timeStatisticsAvailable = !completedRequests.empty();
    averageWaitingTime = sumWaitingTime;
    averageResponseTime = sumResponseTime;
}

std::ostream& operator<<(std::ostream& os, const ServerStats& stats)
{
    os << "Server " << stats.getId() ;
    os << ": Completed=" << stats.getCompletedRequestsCount();
    os << ", Rejected=" << stats.getRejectedRequestsCount();
    os << ", Total=" << stats.getTotalRequestsCount();
    os << ", MaxWaitingQueueSize=" << stats.getMaxWaitingQueueSize();
    os << "\n    Completed=" << stats.getCompletionRate() * 100.0 << '%';
    os << ", Rejected=" << stats.getRejectionRate() * 100.0 << '%';
    os << ", Unfinished=" << stats.getUnprocessedRate() * 100.0 << '%';
    if (stats.timeStatisticsAvailable) {
        os << "\n    Average waiting=" << stats.averageWaitingTime << " s";
        os << ", Max waiting=" << stats.maxWaitingTime << " s";
        os << ", Average response=" << stats.averageResponseTime << " s";
        os << ", Max response=" << stats.maxResponseTime << " s";
    } else {
        os << "\n    Average waiting=N/A, Max waiting=N/A, Average response=N/A, Max response=N/A";
    }
    return os;
}
