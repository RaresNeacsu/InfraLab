#ifndef SIMULATION_TIME_ERROR_H
#define SIMULATION_TIME_ERROR_H

#include "errors/InfraLabError.h"
#include <string>

class SimulationTimeError : public InfraLabError {
public:
    explicit SimulationTimeError(const std::string& message);
};

#endif // SIMULATION_TIME_ERROR_H
