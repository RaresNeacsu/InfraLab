#include <iostream>
#include <fstream>
#include "Simulation.h"
#include "file_loading.h"

int main() {
    try {
        std::ifstream serversFile = loadFromFile("assets/servers.txt");
        std::ifstream requestsFile = loadFromFile("assets/requests.txt");

        std::vector<Server> servers = loadServersFromFile(serversFile);
        std::vector<Request> requests = loadRequestsFromFile(requestsFile);

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
            throw std::runtime_error("Invalid input for time limit. Please enter a valid number.");
        }


        std::cout << "\n\nRunning simulation (time limit: " << timeLimit << " s)...\n";
        Simulation simulation(servers, requests, timeLimit);

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
        std::cout << "\n" << report << std::endl;

        std::ofstream reportFile = saveToFile("assets/simulation_report.txt");
        if (!reportFile) {
            throw std::runtime_error("Failed to create simulation report file.");
        }
        reportFile << report;
        reportFile.close();
        if(!reportFile) {
            throw std::runtime_error("Failed to write simulation report to file.");
        }

        std::cout<< "Unfinished requests - " <<requests.size() - report.getCompletedRequestsCount() - report.getRejectedRequestsCount() <<"/" << requests.size() << " " << std::endl;
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
                    std::cout << request << std::endl;
                }
            }
            std::cout << std::endl;
        }
        std::cout << "Simulation report saved to assets/simulation_report.txt" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
