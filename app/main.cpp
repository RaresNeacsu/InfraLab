#include "errors/ConfigurationError.h"
#include "errors/FileError.h"
#include "errors/InfraLabError.h"
#include "routing/RoundRobinStrategy.h"
#include "routing/LeastLoadedStrategy.h"
#include "routing/FastestServerStrategy.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include "core/Simulation.h"
#include "io/file_loading.h"

int compareConfigurations(const std::string& firstPath, const std::string& secondPath, const std::string& requestsPath);

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--compare") {
        if (argc != 4 && argc != 5) {
            std::cerr << "Usage: oop_main --compare serversA.json serversB.json [requests.json]\n";
            return 1;
        }
        return compareConfigurations(argv[2], argv[3], argc == 5 ? argv[4] : "assets/custom/requests_normal.json");
    }
    try {
        if (argc > 3) {
            throw ConfigurationError("Usage: oop_main [requests.json] [round-robin|least-loaded|fastest]");
        }
        const std::string strategyName = argc == 3 ? argv[2] : "round-robin";
        std::unique_ptr<RoutingStrategy> strategy;
        if (strategyName == "round-robin") strategy = std::make_unique<RoundRobinStrategy>();
        else if (strategyName == "least-loaded") strategy = std::make_unique<LeastLoadedStrategy>();
        else if (strategyName == "fastest") strategy = std::make_unique<FastestServerStrategy>();
        else throw ConfigurationError("Unknown strategy: " + strategyName);
        std::ifstream serversFile = fileLoading::loadFromFile("assets/custom/servers.json");
        std::ifstream requestsFile = fileLoading::loadFromFile(argc > 1 ? argv[1] : "assets/custom/requests_normal.json");

        std::vector<Server> servers = fileLoading::loadServersFromFile(serversFile);
        std::vector<Request> requests = fileLoading::loadRequestsFromFile(requestsFile);

        std::cout << "InfraLab - Infrastructure Simulation\n====================================\n\nLoaded " << servers.size() << " servers and " << requests.size() << " requests." << std::endl;

        std::cout << "\nServers:" << std::endl;
        for (const auto& server : servers) {
            std::cout << server << std::endl;
        }
        std::cout << "\nRequests:" << std::endl;
        for (const auto& request : requests) {
            std::cout << request << std::endl;
        }

        std::cout << "\nInput time limit for the simulation (in seconds): ";
        float timeLimit;
        if(!(std::cin >> timeLimit)){
            throw ConfigurationError("Invalid input for time limit. Please enter a valid number.");
        }


        std::cout << "\n\nRunning simulation (time limit: " << timeLimit << " s)...\n";
        Simulation simulation(servers, requests, timeLimit, std::move(strategy));

        Simulation::StepResult result;
        do {
            result = simulation.step();
            std::cout<<std::endl<<simulation<<std::endl; 
        } while (result == Simulation::StepResult::EventProcessed);

        if(result == Simulation::StepResult::TimeLimitReached) {
            std::cout << "Simulation stopped due to time limit." << std::endl;
        } else if(result == Simulation::StepResult::Completed) {
            std::cout << "Simulation completed all events." << std::endl;
        }
        SimulationReport report = simulation.collectReport();

        std::ofstream reportFile = fileLoading::saveToFile("assets/simulation_report.txt");
        if (!reportFile) {
            throw FileError("Failed to create simulation report file.");
        }
        reportFile << report;
        reportFile<< "Unfinished requests - " <<requests.size() - report.getCompletedRequestsCount() - report.getRejectedRequestsCount() <<"/" << requests.size() << " " << std::endl;
        if(requests.size() - report.getCompletedRequestsCount() - report.getRejectedRequestsCount() > 0){
            for(const auto& request : requests){
                bool completed = false;
                for (const auto& completedRequest : report.getCompletedRequests()) {
                    if (completedRequest.getId() == request.getId()) {
                        completed = true;
                        break;
                    }
                }

                bool rejected = false;
                for (const auto& rejectedRequest : report.getRejectedRequests()) {
                    if (rejectedRequest.getId() == request.getId()) {
                        rejected = true;
                        break;
                    }
                }
                if(!completed && !rejected){
                    reportFile << "Request ID: " << request.getId() << std::endl;
                }
            }
            reportFile << std::endl;
        }
        reportFile.close();
        if(!reportFile) {
            throw FileError("Failed to write simulation report to file.");
        }

        std::cout << "Simulation report saved to assets/simulation_report.txt" << std::endl;
    } catch (const FileError& e) {
        std::cerr << "File error: " << e.what() << std::endl;
        return 1;
    } catch (const InfraLabError& e) {
        std::cerr << "InfraLab error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
