#include "errors/ConfigurationError.h"

ConfigurationError::ConfigurationError(const std::string& message)
    : InfraLabError(message) {}
