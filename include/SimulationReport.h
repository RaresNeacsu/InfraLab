#ifndef SIMULATION_REPORT_H
#define SIMULATION_REPORT_H
#include <ostream>
#include <vector>
#include "Request.h"

class SimulationReport
{
    public:
        SimulationReport(const std::vector<Request>& completedRequests, const std::vector<Request>& rejectedRequests);
        float averageWaitingTime() const;
        float averageResponseTime() const;
        int getCompletedRequestsCount() const { return completedRequests.size(); }
        int getRejectedRequestsCount() const { return rejectedRequests.size(); }
        const std::vector<Request>& getCompletedRequests() const { return completedRequests; }
        const std::vector<Request>& getRejectedRequests() const { return rejectedRequests; }
        friend std::ostream& operator<<(std::ostream& os, const SimulationReport& report);
    private:
        std::vector<Request> completedRequests;
        std::vector<Request> rejectedRequests;
};

#endif // SIMULATION_REPORT_H