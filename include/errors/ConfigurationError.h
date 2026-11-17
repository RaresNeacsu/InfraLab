#ifndef CONFIGURATION_ERROR_H
#define CONFIGURATION_ERROR_H

#include "errors/InfraLabError.h"
#include <string>

class ConfigurationError : public InfraLabError {
public:
    explicit ConfigurationError(const std::string& message);
};

#endif // CONFIGURATION_ERROR_H
