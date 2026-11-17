#ifndef REPORT_ERROR_H
#define REPORT_ERROR_H

#include "errors/InfraLabError.h"
#include <string>

class ReportError : public InfraLabError {
public:
    explicit ReportError(const std::string& message);
};

#endif // REPORT_ERROR_H
