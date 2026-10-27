#pragma once

#include <fstream>
#include <string>
#include <stdexcept>
#include "Server.h"
#include "Request.h"
#include <vector>

std::ifstream loadFromFile(const std::string& filename);
std::ofstream saveToFile(const std::string& filename);
std::vector<Request> loadRequestsFromFile(std::ifstream &file);
std::vector<Server> loadServersFromFile(std::ifstream &file);
