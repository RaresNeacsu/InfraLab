#include <iostream>
#include <string>
#include "ComparisonReport.h"
#include "Simulation.h"
#include "file_loading.h"

static SimulationReport runConfiguration(const std::vector<Server>& servers, const std::vector<Request>& requests, double timeLimit, const char* label)
{
    std::cout << "\nRunning configuration " << label << '\n';
    Simulation simulation(servers, requests, timeLimit);
    Simulation::StepResult result;
    do {
        result = simulation.step();
        std::cout << '\n' << simulation << '\n';
    } while (result == Simulation::StepResult::EventProcessed);
    std::cout << "Configuration " << label << (result == Simulation::StepResult::Completed
        ? " completed all events.\n" : " reached the time limit.\n");
    return simulation.collectReport();
}

int compareConfigurations(const std::string& firstPath, const std::string& secondPath, const std::string& requestsPath)
{
    try {
        auto firstFile = fileLoading::loadFromFile(firstPath);
        auto secondFile = fileLoading::loadFromFile(secondPath);
        auto trafficFile = fileLoading::loadFromFile(requestsPath);
        const auto firstServers = fileLoading::loadServersFromFile(firstFile);
        const auto secondServers = fileLoading::loadServersFromFile(secondFile);
        const auto requests = fileLoading::loadRequestsFromFile(trafficFile);
        std::cout << "Input time limit for both simulations (in seconds): ";
        double timeLimit = 0.0;
        if (!(std::cin >> timeLimit)) {
            throw std::runtime_error("Invalid input for time limit.");
        }
        const auto first = runConfiguration(firstServers, requests, timeLimit, "A");
        const auto second = runConfiguration(secondServers, requests, timeLimit, "B");
        const ComparisonReport comparison(first, second, requests.size(), timeLimit);
        auto output = fileLoading::saveToFile("assets/comparison_report.txt");
        output << "Configuration A: " << firstPath << "\nConfiguration B: " << secondPath
               << "\nTraffic: " << requestsPath << "\n\n" << comparison;
        output.close();
        if (!output) throw std::runtime_error("Failed to write comparison report.");
        std::cout << "Comparison saved to assets/comparison_report.txt\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Comparison error: " << error.what() << '\n';
        return 1;
    }
}
