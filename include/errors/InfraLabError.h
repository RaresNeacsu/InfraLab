#ifndef INFRALAB_ERROR_H
#define INFRALAB_ERROR_H

#include <string>
#include <stdexcept>

class InfraLabError : public std::runtime_error {
  public:
    explicit InfraLabError(const std::string& message);

};

#endif // INFRALAB_ERROR_H