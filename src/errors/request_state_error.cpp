#include "errors/RequestStateError.h"

RequestStateError::RequestStateError(const std::string& message)
    : InfraLabError(message)
{
}
