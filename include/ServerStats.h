#ifndef ServerStats_H
#define ServerStats_H
#include <ostream>
#include <vector>
#include "Request.h"

class ServerStats
{
    public:
        ServerStats(int id, int completedRequestsCount, int rejectedRequestsCount, int totalRequestsCount, int maxWaitingQueueSize);
        int getId() const { return id; }
        int getCompletedRequestsCount() const { return completedRequestsCount; }
        int getRejectedRequestsCount() const { return rejectedRequestsCount; }
        int getTotalRequestsCount() const { return totalRequestsCount; }
        int getMaxWaitingQueueSize() const { return maxWaitingQueueSize; }
        // Fractions in [0, 1]; an idle server returns zero for every rate.
        double getRejectionRate() const;
        double getCompletionRate() const;
        double getUnprocessedRate() const;
        void calculateTimeStatistics(const std::vector<Request>& completedRequests);

        bool hasTimeStatistics() const { return timeStatisticsAvailable; }
        double getAverageWaitingTime() const;
        friend std::ostream& operator<<(std::ostream& os, const ServerStats& stats);
    private:
        int id;
        int completedRequestsCount;
        int rejectedRequestsCount;
        int totalRequestsCount;
        int maxWaitingQueueSize;
        double maxWaitingTime = 0.0;
        double maxResponseTime = 0.0;
        double averageWaitingTime = 0.0;
        double averageResponseTime = 0.0;
        bool timeStatisticsAvailable = false;

};

#endif // ServerStats_H
