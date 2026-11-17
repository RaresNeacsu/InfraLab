#include "errors/InfraLabError.h"
#include <string>

InfraLabError::InfraLabError(const std::string& message)
    : std::runtime_error(message) {}