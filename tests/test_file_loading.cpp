#undef NDEBUG
#include <cassert>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "file_loading.h"
#include "file_loading.h"




void testFileLoading()
{
    std::ifstream serversJson = loadFromFile("assets/servers.json");
    std::ifstream requestsJson = loadFromFile("assets/requests.json");
    const auto servers = loadServersFromFile(serversJson);
    const auto requests = loadRequestsFromFile(requestsJson);

    assert(servers.size() == 4);
    assert(servers[0].getId() == 1);
    assert(servers[0].getProcessingSpeed() == 1.0f);
    assert(servers[0].getWaitingQueueLimit() == 2);
    assert(servers[3].getId() == 4);
    assert(servers[3].getCapacity() == 3);
    assert(requests.size() == 24);
    assert(requests[0].getId() == 1);
    assert(requests[0].getArrivalTime() == 0.0f);
    assert(requests[0].getWorkUnit() == 2.0f);
    assert(requests[23].getId() == 24);
    assert(requests[23].getArrivalTime() == 8.0f);
    assert(requests[23].getWorkUnit() == 3.0f);

    for (const char* invalid : {
        R"([{"id":1.5,"arrival_time":0,"work_units":1}])",
        R"([{"id":2147483648,"arrival_time":0,"work_units":1}])",
        R"([{"id":-2147483649,"arrival_time":0,"work_units":1}])"}) {
        std::istringstream input(invalid);
        bool caught = false;
        try { loadRequestsFromFile(input); }
        catch (const std::exception&) { caught = true; }
        assert(caught);
    }
    for (const char* invalid : {
        R"([{"id":1.5,"processing_speed":1,"waiting_queue_limit":1,"capacity":1}])",
        R"([{"id":1,"processing_speed":1,"waiting_queue_limit":1.5,"capacity":1}])",
        R"([{"id":1,"processing_speed":1,"waiting_queue_limit":1,"capacity":1.5}])",
        R"([{"id":1,"processing_speed":1,"waiting_queue_limit":1,"capacity":2147483648}])",
        R"([{"id":1,"processing_speed":1,"waiting_queue_limit":-2147483649,"capacity":1}])"}) {
        std::istringstream input(invalid);
        bool caught = false;
        try { loadServersFromFile(input); }
        catch (const std::exception&) { caught = true; }
        assert(caught);
    }
    // Malformed JSON, missing fields, and non-array roots must be rejected.
    for (const char* invalid : {
        "[{",
        R"([{"id":1,"arrival_time":0}])",
        R"({"id":1,"arrival_time":0,"work_units":1})"}) {
        std::istringstream input(invalid);
        bool caught = false;
        try { loadRequestsFromFile(input); }
        catch (const std::exception&) { caught = true; }
        assert(caught);
    }
    for (const char* invalid : {
        "[{",
        R"([{"id":1,"processing_speed":1,"capacity":1}])",
        R"({"id":1,"processing_speed":1,"waiting_queue_limit":0,"capacity":1})"}) {
        std::istringstream input(invalid);
        bool caught = false;
        try { loadServersFromFile(input); }
        catch (const std::exception&) { caught = true; }
        assert(caught);
    }
    // Constructors reject invalid domain values after JSON parsing.
    std::istringstream invalidRequest(R"([{"id":1,"arrival_time":0,"work_units":0}])");
    bool requestRejected = false;
    try { loadRequestsFromFile(invalidRequest); }
    catch (const std::invalid_argument&) { requestRejected = true; }
    assert(requestRejected);
    std::istringstream invalidServer(R"([{"id":1,"processing_speed":1,"waiting_queue_limit":0,"capacity":0}])");
    bool serverRejected = false;
    try { loadServersFromFile(invalidServer); }
    catch (const std::invalid_argument&) { serverRejected = true; }
    assert(serverRejected);
    // An ID identifies one request even when its data differs.
    std::istringstream duplicates(R"([{"id":1,"arrival_time":0,"work_units":1},{"id":1,"arrival_time":1,"work_units":2}])");
    bool duplicateRejected = false;
    try { loadRequestsFromFile(duplicates); }
    catch (const std::invalid_argument&) { duplicateRejected = true; }
    assert(duplicateRejected);
    std::istringstream distinct(R"([{"id":1,"arrival_time":0,"work_units":1},{"id":2,"arrival_time":0,"work_units":1}])");
    assert(loadRequestsFromFile(distinct).size() == 2);
    std::istringstream valid(R"([{"id":1,"processing_speed":1,"waiting_queue_limit":0,"capacity":1}])");
    assert(loadServersFromFile(valid).size() == 1);


}