#undef NDEBUG
#include <cassert>
#include <exception>
#include <sstream>
#include <stdexcept>

#include "file_loading.h"

namespace {
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
}
