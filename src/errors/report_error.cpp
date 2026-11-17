#include "errors/ReportError.h"

ReportError::ReportError(const std::string& message)
    : InfraLabError(message)
{
}
