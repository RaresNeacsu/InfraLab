#include "file_loading.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "Server.h"
#include "Request.h"


std::ifstream loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    return file;
}
std::ofstream saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    return file;
}
std::vector<Server> loadServersFromFile(std::ifstream &file) {
    std::string header;
    std::getline(file, header);
    std::vector<Server> servers;
    int id;
    float processingSpeed;
    int waitingQueueLimit;
    int capacity;
    std::string line;
    int lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        std::istringstream row(line);
        if (!(row >> id >> processingSpeed >> waitingQueueLimit >> capacity) ||
            !(row >> std::ws).eof()) {
            throw std::runtime_error("Invalid server data on line " + std::to_string(lineNumber) +
                ": expected int float int int.");
        }
        servers.push_back(Server(id, processingSpeed, waitingQueueLimit, capacity));
    }
    return servers;
}
std::vector<Request> loadRequestsFromFile(std::ifstream &file) {
    std::string header;
    std::getline(file, header);
    std::vector<Request> requests;
    int id;
    float arrivalTime;
    float workUnit;
    std::string line;
    int lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        std::istringstream row(line);
        if (!(row >> id >> arrivalTime >> workUnit) ||
            !(row >> std::ws).eof()) {
            throw std::runtime_error("Invalid request data on line " + std::to_string(lineNumber) +
                ": expected int float float.");
        }
        requests.push_back(Request(id, arrivalTime, workUnit));
    }
    return requests;
}