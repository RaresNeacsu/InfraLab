#undef NDEBUG
#include <cassert>
#include <exception>
#include <filesystem>
#include <chrono>
#include <system_error>
#include <sstream>
#include <stdexcept>

#include "file_loading.h"

namespace {
class TemporaryDirectory {
public:
    TemporaryDirectory()
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (int attempt = 0; attempt < 100; ++attempt) {
            directory = std::filesystem::temp_directory_path() /
                ("infralab-file-tests-" + std::to_string(stamp) + "-" + std::to_string(attempt));
            if (std::filesystem::create_directory(directory)) return;
        }
        throw std::runtime_error("Could not create a temporary test directory.");
    }
    ~TemporaryDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }
    const std::filesystem::path& path() const { return directory; }
private:
    std::filesystem::path directory;
};

template <typename Exception, typename Operation>
void expectException(Operation operation)
{
    bool caught = false;
    try {
        operation();
    } catch (const Exception&) {
        caught = true;
    }
    assert(caught && "Expected file loading to reject invalid input");
}
}

void testFileLoading()
{
    std::istringstream serversInput(R"([
        {"id":7,"processing_speed":2.5,"waiting_queue_limit":0,"capacity":3},
        {"id":9,"processing_speed":1,"waiting_queue_limit":4,"capacity":1}
    ])");
    const auto servers = fileLoading::loadServersFromFile(serversInput);
    assert(servers.size() == 2);
    assert(servers[0].getId() == 7);
    assert(servers[0].getProcessingSpeed() == 2.5f);
    assert(servers[0].getWaitingQueueLimit() == 0);
    assert(servers[0].getCapacity() == 3);
    assert(servers[0].getActiveQueueSize() == 0);
    assert(servers[0].getWaitingQueueSize() == 0);
    assert(servers[1].getId() == 9);
    assert(servers[1].getProcessingSpeed() == 1.0f);
    assert(servers[1].getWaitingQueueLimit() == 4);
    assert(servers[1].getCapacity() == 1);

    std::istringstream requestsInput(R"([
        {"id":8,"arrival_time":0,"work_units":0.25},
        {"id":2,"arrival_time":1.5,"work_units":4}
    ])");
    const auto requests = fileLoading::loadRequestsFromFile(requestsInput);
    assert(requests.size() == 2);
    assert(requests[0].getId() == 8);
    assert(requests[0].getArrivalTime() == 0.0f);
    assert(requests[0].getWorkUnit() == 0.25f);
    assert(requests[0].getState() == Request::State::PENDING);
    assert(requests[1].getId() == 2);
    assert(requests[1].getArrivalTime() == 1.5f);
    assert(requests[1].getWorkUnit() == 4.0f);
    assert(requests[1].getState() == Request::State::PENDING);

    std::istringstream emptyServers("[]");
    const auto noServers = fileLoading::loadServersFromFile(emptyServers);
    assert(noServers.empty());
    std::istringstream emptyRequests("[]");
    const auto noRequests = fileLoading::loadRequestsFromFile(emptyRequests);
    assert(noRequests.empty());

    // Parsing errors belong to the private JSON dependency; test rejection
    // through the public interface without including that dependency here.
    for (const char* invalidJson : {"", "[", "[{}", "not json", "{}", "null", "42"}) {
        expectException<std::exception>([&] {
            std::istringstream input(invalidJson);
            fileLoading::loadServersFromFile(input);
        });
        expectException<std::exception>([&] {
            std::istringstream input(invalidJson);
            fileLoading::loadRequestsFromFile(input);
        });
    }

    for (const char* invalidServer : {
        R"([{"id":1,"waiting_queue_limit":0,"capacity":1}])",
        R"([{"id":1,"processing_speed":"fast","waiting_queue_limit":0,"capacity":1}])",
        R"([{"id":1,"processing_speed":1,"waiting_queue_limit":0,"capacity":null}])"
    }) {
        expectException<std::exception>([&] {
            std::istringstream input(invalidServer);
            fileLoading::loadServersFromFile(input);
        });
    }

    for (const char* invalidRequest : {
        R"([{"id":1,"arrival_time":0}])",
        R"([{"id":1,"arrival_time":"later","work_units":1}])",
        R"([{"id":1,"arrival_time":0,"work_units":null}])"
    }) {
        expectException<std::exception>([&] {
            std::istringstream input(invalidRequest);
            fileLoading::loadRequestsFromFile(input);
        });
    }

    for (const char* invalidServer : {
        R"([{"id":0,"processing_speed":1,"waiting_queue_limit":0,"capacity":1}])",
        R"([{"id":1,"processing_speed":0,"waiting_queue_limit":0,"capacity":1}])",
        R"([{"id":1,"processing_speed":1,"waiting_queue_limit":-1,"capacity":1}])",
        R"([{"id":1,"processing_speed":1,"waiting_queue_limit":0,"capacity":0}])",
        R"([{"id":1,"processing_speed":1,"waiting_queue_limit":0,"capacity":1.5}])"
    }) {
        expectException<std::invalid_argument>([&] {
            std::istringstream input(invalidServer);
            fileLoading::loadServersFromFile(input);
        });
    }

    for (const char* invalidRequest : {
        R"([{"id":0,"arrival_time":0,"work_units":1}])",
        R"([{"id":1,"arrival_time":-1,"work_units":1}])",
        R"([{"id":1,"arrival_time":0,"work_units":0}])",
        R"([{"id":1.5,"arrival_time":0,"work_units":1}])",
        R"([{"id":true,"arrival_time":0,"work_units":1}])",
        R"([{"id":1,"arrival_time":0,"work_units":1},
            {"id":1,"arrival_time":2,"work_units":3}])"
    }) {
        expectException<std::invalid_argument>([&] {
            std::istringstream input(invalidRequest);
            fileLoading::loadRequestsFromFile(input);
        });
    }

    // Both signed and unsigned JSON integers must fit the public int ID type.
    for (const char* id : {"9223372036854775807", "18446744073709551615", "-9223372036854775808"}) {
        expectException<std::out_of_range>([&] {
            std::istringstream input(std::string("[{\"id\":") + id +
                ",\"arrival_time\":0,\"work_units\":1}]");
            fileLoading::loadRequestsFromFile(input);
        });
        expectException<std::out_of_range>([&] {
            std::istringstream input(std::string("[{\"id\":") + id +
                ",\"processing_speed\":1,\"waiting_queue_limit\":0,\"capacity\":1}]");
            fileLoading::loadServersFromFile(input);
        });
    }
    expectException<std::invalid_argument>([] {
        std::istringstream input(R"([
            {"id":1,"processing_speed":1,"waiting_queue_limit":0,"capacity":1},
            {"id":1,"processing_speed":2,"waiting_queue_limit":0,"capacity":1}
        ])");
        fileLoading::loadServersFromFile(input);
    });
    for (const char* invalidNumeric : {
        R"([{"id":1,"arrival_time":true,"work_units":1}])",
        R"([{"id":1,"arrival_time":0,"work_units":true}])"
    }) {
        expectException<std::invalid_argument>([&] {
            std::istringstream input(invalidNumeric);
            fileLoading::loadRequestsFromFile(input);
        });
    }
    expectException<std::invalid_argument>([] {
        std::istringstream input(R"([{"id":1,"processing_speed":true,"waiting_queue_limit":0,"capacity":1}])");
        fileLoading::loadServersFromFile(input);
    });
    TemporaryDirectory temporary;
    const auto missing = (temporary.path() / "missing.json").string();
    expectException<std::runtime_error>([&] { fileLoading::loadFromFile(missing); });
    const auto requestPath = (temporary.path() / "requests.json").string();
    {
        auto output = fileLoading::saveToFile(requestPath);
        output << R"([{"id":17,"arrival_time":0.25,"work_units":1.5}])";
        output.close();
        assert(output);
    }
    {
        auto input = fileLoading::loadFromFile(requestPath);
        const auto loadedRequests = fileLoading::loadRequestsFromFile(input);
        assert(loadedRequests.size() == 1);
        assert(loadedRequests[0].getId() == 17);
        assert(loadedRequests[0].getArrivalTime() == 0.25);
        assert(loadedRequests[0].getWorkUnit() == 1.5);
    }
    expectException<std::runtime_error>([&] {
        fileLoading::saveToFile((temporary.path() / "missing-parent" / "report.txt").string());
    });
    {
        auto output = fileLoading::saveToFile(requestPath);
        output << "[]";
        output.close();
        assert(output);
        auto input = fileLoading::loadFromFile(requestPath);
        const auto overwrittenRequests = fileLoading::loadRequestsFromFile(input);
        assert(overwrittenRequests.empty());
    }

}
