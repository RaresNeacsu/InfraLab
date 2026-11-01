#include <fstream>
#include <limits>
#include <cmath>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <unordered_set>
#include "file_loading.h"
#include "Server.h"
#include "Request.h"
namespace {
double readNumber(const nlohmann::json& item, const char* field) {
    const auto& value = item.at(field);
    if (!value.is_number()) {
        throw std::invalid_argument(std::string(field) + " must be a number.");
    }
    const double result = value.get<double>();
    if (!std::isfinite(result)) {
        throw std::out_of_range(std::string(field) + " must be finite.");
    }
    return result;
}

int readInteger(const nlohmann::json& item, const char* field) {
    const auto& value = item.at(field);
    if (!value.is_number_integer()) {
        throw std::invalid_argument(std::string(field) + " must be an integer.");
    }
    if (value.is_number_unsigned()) {
        if (value.get<nlohmann::json::number_unsigned_t>() > static_cast<nlohmann::json::number_unsigned_t>(std::numeric_limits<int>::max())) {
            throw std::out_of_range(std::string(field) + " is outside the int range.");
        }
    } else {
        const auto number = value.get<nlohmann::json::number_integer_t>();
        if (number < std::numeric_limits<int>::min() || number > std::numeric_limits<int>::max()) {
            throw std::out_of_range(std::string(field) + " is outside the int range.");
        }
    }
    return value.get<int>();
    }
}


std::ifstream fileLoading::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    return file;
}
std::ofstream fileLoading::saveToFile(const std::string& filename){
    std::ofstream file(filename);
    if(!file.is_open()){
        throw std::runtime_error("Could not open file: " + filename);
    }
    return file;
}

std::vector<Server> fileLoading::loadServersFromFile(std::istream &file) {
    nlohmann::json data;
    file >> data;
    if (!data.is_array()) {
        throw std::runtime_error("Server data must be a JSON array.");
    }

    std::vector<Server> servers;
    std::unordered_set<int> serverIds;
    for (const auto& item : data) {
        const int id = readInteger(item, "id");
        if (!serverIds.insert(id).second) {
            throw std::invalid_argument("Duplicate server ID: " + std::to_string(id));
        }
        servers.emplace_back(id,
                             readNumber(item, "processing_speed"),
                             readInteger(item, "waiting_queue_limit"),
                             readInteger(item, "capacity"));
    }
    return servers;
}

std::vector<Request> fileLoading::loadRequestsFromFile(std::istream &file) {
    nlohmann::json data;
    file >> data;
    if (!data.is_array()) {
        throw std::runtime_error("Request data must be a JSON array.");
    }

    std::vector<Request> requests;
    std::unordered_set<int> requestIds;
    for (const auto& item : data) {
        const int id = readInteger(item, "id");
        if (!requestIds.insert(id).second) {
            throw std::invalid_argument("Duplicate request ID: " + std::to_string(id));
        }
        requests.emplace_back(id,
                              readNumber(item, "arrival_time"),
                              readNumber(item, "work_units"));
    }
    return requests;
}
