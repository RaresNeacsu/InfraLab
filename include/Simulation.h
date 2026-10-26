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
    Simulation(const std::vector<Server> &servers, const std::vector<Request> &incomingRequests, float maximumTime);
    float nextEventTime() const;
    StepResult step();
    SimulationReport collectReport();
    ~Simulation() = default;
    friend std::ostream& operator<<(std::ostream& os, const Simulation& simulation);

private:
    std::vector<Server> servers;
    std::vector<Request> incomingRequests;
    std::size_t nextArrivalIndex = 0;
    std::size_t nextServerIndex = 0;
    float currentTime = 0.0f;
    float maximumTime;
};
#endif // SIMULATION_H

std::ostream& operator<<(std::ostream& os, const Simulation& simulation);