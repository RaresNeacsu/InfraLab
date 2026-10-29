#ifndef FILE_LOADING_H
#define FILE_LOADING_H

#include <fstream>
#include <string>
#include <stdexcept>
#include "Server.h"
#include "Request.h"
#include <vector>

std::ifstream loadFromFile(const std::string& filename);
std::ofstream saveToFile(const std::string& filename);
std::vector<Request> loadRequestsFromFile(std::istream &file);
std::vector<Server> loadServersFromFile(std::istream &file);

#endif // FILE_LOADING_H
