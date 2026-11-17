
#include "errors/FileError.h"
#include <string>

FileError::FileError(const std::string& message)
    : InfraLabError(message) {}