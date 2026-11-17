#ifndef FILE_ERROR_H
#define FILE_ERROR_H
#include "errors/InfraLabError.h"
#include <string>

class FileError : public InfraLabError {
  public:
    explicit FileError(const std::string& message);
};

#endif // FILE_ERROR_H