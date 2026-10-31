#include "ServerStats.h"
#include <stdexcept>

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

std::ostream& operator<<(std::ostream& os, const ServerStats& stats)
{
    os << "Server " << stats.getId() ;
    os << ": Completed=" << stats.getCompletedRequestsCount();
    os << ", Rejected=" << stats.getRejectedRequestsCount();
    os << ", Total=" << stats.getTotalRequestsCount();
    os << ", MaxWaitingQueueSize=" << stats.getMaxWaitingQueueSize();
    return os;
}
