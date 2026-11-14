#ifndef SIMULATION_H
#define SIMULATION_H
#include "Server.h"
#include "Request.h"
#include "SimulationReport.h"
#include "RoutingStrategy.h"
#include <memory>
#include <vector>
#include <cstddef>

class Simulation
{
public:
    enum class StepResult
    {
        EventProcessed,
        Completed,
        TimeLimitReached,
    };
    Simulation(const std::vector<Server> &servers, const std::vector<Request> &incomingRequests, double maximumTime, std::unique_ptr<RoutingStrategy> strategy);
    Simulation(const Simulation& other);
    Simulation& operator=(const Simulation& other);
    StepResult step();
    // Cumulative snapshot: collecting a report does not consume simulation history.
    SimulationReport collectReport() const;
    ~Simulation() = default;
    friend std::ostream& operator<<(std::ostream& os, const Simulation& simulation);

private:
    void swap(Simulation& other) noexcept;
    double nextEventTime() const;
    std::vector<Server> servers;
    std::vector<Request> incomingRequests;
    std::vector<Request> globallyRejectedRequests;
    std::size_t nextArrivalIndex = 0;
    double currentTime = 0.0f;
    double maximumTime;
    std::unique_ptr<RoutingStrategy> routingStrategy;
};
#endif // SIMULATION_H

std::ostream& operator<<(std::ostream& os, const Simulation& simulation);
