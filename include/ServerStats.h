#ifndef ServerStats_H
#define ServerStats_H
#include <ostream>

class ServerStats
{
    public:
        ServerStats(int id, int completedRequestsCount, int rejectedRequestsCount, int totalRequestsCount, int maxWaitingQueueSize);
        int getId() const { return id; }
        int getCompletedRequestsCount() const { return completedRequestsCount; }
        int getRejectedRequestsCount() const { return rejectedRequestsCount; }
        int getTotalRequestsCount() const { return totalRequestsCount; }
        int getMaxWaitingQueueSize() const { return maxWaitingQueueSize; }
        friend std::ostream& operator<<(std::ostream& os, const ServerStats& stats);
    private:
        int id;
        int completedRequestsCount;
        int rejectedRequestsCount;
        int totalRequestsCount;
        int maxWaitingQueueSize;
        
};

#endif // ServerStats_H
