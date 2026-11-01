#ifndef SIMULATION_H
#define SIMULATION_H
#include "Server.h"
#include "Request.h"
#include "SimulationReport.h"
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
    Simulation(const std::vector<Server> &servers, const std::vector<Request> &incomingRequests, double maximumTime);
    StepResult step();
    // Cumulative snapshot: collecting a report does not consume simulation history.
    SimulationReport collectReport() const;
    ~Simulation() = default;
    friend std::ostream& operator<<(std::ostream& os, const Simulation& simulation);

private:
    double nextEventTime() const;
    std::vector<Server> servers;
    std::vector<Request> incomingRequests;
    std::size_t nextArrivalIndex = 0;
    std::size_t nextServerIndex = 0;
    double currentTime = 0.0f;
    double maximumTime;
    
};
#endif // SIMULATION_H

std::ostream& operator<<(std::ostream& os, const Simulation& simulation);
