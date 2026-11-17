#ifndef REQUEST_STATE_ERROR_H
#define REQUEST_STATE_ERROR_H

#include "errors/InfraLabError.h"
#include <string>

class RequestStateError : public InfraLabError {
public:
    explicit RequestStateError(const std::string& message);
};

#endif // REQUEST_STATE_ERROR_H
