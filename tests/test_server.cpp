#undef NDEBUG
#include <array>
#include <cassert>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Server.h"

void testServer()
{
    bool caught = false;
    // test constructor and getters ///////////////////////////////////////////
    const Server initial(3, 2.5f, 2, 1);
    assert(initial.getId() == 3);
    assert(initial.getProcessingSpeed() == 2.5f);
    assert(initial.getWaitingQueueLimit() == 2);
    assert(initial.getCapacity() == 1);
    assert(initial.getActiveQueueSize() == 0);
    assert(initial.getWaitingQueueSize() == 0);
    assert(initial.getCompletedQueueSize() == 0);

    caught = false;
    try
    {
        Server invalid(0, 2.5f, 2, 1);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");
    caught = false;
    try
    {
        Server invalid(-1, 2.5f, 2, 1);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");
    caught = false;
    try
    {
        Server invalid(1, 2.5f, 2, 0);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");
    caught = false;
    try
    {
        Server invalid(1, 2.5f, 2, -1);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");
    caught = false;
    try
    {
        Server invalid(1, 0.0f, 2, 1);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");
    caught = false;
    try
    {
        Server invalid(1, -1.0f, 2, 1);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");
    caught = false;
    try
    {
        Server invalid(1, std::numeric_limits<float>::quiet_NaN(), 2, 1);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");
    caught = false;
    try
    {
        Server invalid(1, std::numeric_limits<float>::infinity(), 2, 1);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");
    caught = false;
    try
    {
        Server invalid(1, -std::numeric_limits<float>::infinity(), 2, 1);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");
    caught = false;
    try
    {
        Server invalid(1, 2.5f, -1, 1);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "Expected invalid Server parameters to throw");

    // test submitRequest() - direct admission and rejection ///////////////////////////////////////////
    Request first(1, 1.0f, 4);
    Request second(2, 1.0f, 1);
    Server direct(4, 2.0f, 0, 1);
    bool accepted = direct.submitRequest(first, 1.25f);
    assert(accepted);
    assert(direct.getActiveQueueSize() == 1);
    assert(direct.getWaitingQueueSize() == 0);
    assert(first.getState() == Request::State::PENDING);
    assert(first.getStartTime() == -1.0f);
    accepted = direct.submitRequest(second, 1.25f);
    assert(!accepted && "Full server with no waiting slots must refuse admission");
    assert(second.getState() == Request::State::PENDING && "Refusal must modify only the server copy");
    assert(direct.getActiveQueueSize() == 1);
    assert(direct.getWaitingQueueSize() == 0);
    assert(direct.getCompletedQueueSize() == 0);
    direct.completeActiveRequest(3.0f);
    assert(direct.getActiveQueueSize() == 1);
    direct.completeActiveRequest(3.25f);
    assert(direct.getActiveQueueSize() == 0);
    assert(direct.getCompletedQueueSize() == 1);
    accepted = direct.submitRequest(second, 3.25f);
    assert(accepted && "Completed requests must release processing capacity");
    direct.completeActiveRequest(3.75f);
    assert(direct.getCompletedQueueSize() == 2);

    // test submitRequest() - waiting queue and FIFO priority ///////////////////////////////////////////
    Server fifo(5, 2.0f, 1, 1);
    accepted = fifo.submitRequest(first, 1.0f);
    assert(accepted);
    accepted = fifo.submitRequest(second, 1.0f);
    assert(accepted);
    assert(fifo.getActiveQueueSize() == 1);
    assert(fifo.getWaitingQueueSize() == 1);
    Request newcomer(3, 1.0f, 8);
    accepted = fifo.submitRequest(newcomer, 1.0f);
    assert(!accepted);
    fifo.waitingQueueToActive(1.5f);
    assert(fifo.getWaitingQueueSize() == 1);
    assert(fifo.getActiveQueueSize() == 1);
    fifo.completeActiveRequest(3.0f);
    assert(fifo.getCompletedQueueSize() == 1);
    assert(fifo.getActiveQueueSize() == 0);
    assert(fifo.getWaitingQueueSize() == 1);
    accepted = fifo.submitRequest(newcomer, 3.0f);
    assert(accepted && "Admission must first give available capacity to older waiting Requests");
    assert(fifo.getActiveQueueSize() == 1);
    assert(fifo.getWaitingQueueSize() == 1);
    fifo.completeActiveRequest(3.5f);
    assert(fifo.getCompletedQueueSize() == 2 && "Older short Request must run before the newcomer");
    assert(fifo.getActiveQueueSize() == 0);
    fifo.waitingQueueToActive(3.5f);
    assert(fifo.getWaitingQueueSize() == 0);
    assert(fifo.getActiveQueueSize() == 1);
    fifo.completeActiveRequest(7.5f);
    assert(fifo.getCompletedQueueSize() == 3);

    // test submitRequest() - invalid arguments and states ///////////////////////////////////////////
    const std::array<float, 4> invalidTimes = {
        -1.0f, std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()
    };
    Server validation(6, 2.0f, 1, 1);
    for (float time : invalidTimes)
    {
        caught = false;
        try
        {
            validation.submitRequest(first, time);
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }
        assert(caught && "Invalid admission time must throw");
        assert(validation.getWaitingQueueSize() == 0);
        assert(validation.getActiveQueueSize() == 0);
        assert(validation.getCompletedQueueSize() == 0);
    }
    caught = false;
    try
    {
        validation.submitRequest(first, 0.5f);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught && "A Request must not be admitted before arrival");
    Request started(4, 0.0f, 1);
    started.start(0.0f);
    Request completed = started;
    completed.complete(1.0f);
    Request rejected(5, 0.0f, 1);
    rejected.reject();
    const std::array<Request, 3> invalidRequests = {started, completed, rejected};
    for (const Request& request : invalidRequests)
    {
        caught = false;
        try
        {
            validation.submitRequest(request, 1.0f);
        }
        catch (const std::runtime_error&)
        {
            caught = true;
        }
        assert(caught && "Only pending Requests may be admitted");
        assert(validation.getWaitingQueueSize() == 0);
        assert(validation.getActiveQueueSize() == 0);
    }
    accepted = validation.submitRequest(first, 1.0f);
    assert(accepted && "Valid admission must work after failed admissions");
    accepted = validation.submitRequest(second, 1.0f);
    assert(accepted);
    for (float time : invalidTimes)
    {
        caught = false;
        try
        {
            validation.submitRequest(newcomer, time);
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }
        assert(caught && "Invalid admission time must throw even when full");
        assert(validation.getActiveQueueSize() == 1);
        assert(validation.getWaitingQueueSize() == 1);
    }

    // test waitingQueueToActive() ///////////////////////////////////////////
    Server transfer(7, 2.0f, 2, 2);
    transfer.waitingQueueToActive(0.0f);
    assert(transfer.getActiveQueueSize() == 0);
    accepted = transfer.submitRequest(first, 1.0f);
    assert(accepted);
    accepted = transfer.submitRequest(first, 1.0f);
    assert(accepted);
    accepted = transfer.submitRequest(second, 1.0f);
    assert(accepted);
    accepted = transfer.submitRequest(newcomer, 1.0f);
    assert(accepted);
    transfer.completeActiveRequest(3.0f);
    assert(transfer.getActiveQueueSize() == 0);
    assert(transfer.getCompletedQueueSize() == 2);
    assert(transfer.getWaitingQueueSize() == 2);
    for (float time : invalidTimes)
    {
        caught = false;
        try
        {
            transfer.waitingQueueToActive(time);
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }
        assert(caught && "Invalid start time must preserve waiting Requests");
        assert(transfer.getWaitingQueueSize() == 2);
        assert(transfer.getActiveQueueSize() == 0);
        assert(transfer.getCompletedQueueSize() == 2);
    }
    transfer.waitingQueueToActive(3.0f);
    assert(transfer.getWaitingQueueSize() == 0);
    assert(transfer.getActiveQueueSize() == 2 && "Transfer must fill all available slots");
    transfer.completeActiveRequest(3.5f);
    assert(transfer.getActiveQueueSize() == 1);
    assert(transfer.getCompletedQueueSize() == 3);
    transfer.completeActiveRequest(7.0f);
    assert(transfer.getActiveQueueSize() == 0);
    assert(transfer.getCompletedQueueSize() == 4);

    // test completeActiveRequest() - concurrent requests ///////////////////////////////////////////
    Server concurrent(8, 2.0f, 0, 2);
    accepted = concurrent.submitRequest(first, 1.0f);
    assert(accepted);
    accepted = concurrent.submitRequest(second, 1.0f);
    assert(accepted);
    for (float time : invalidTimes)
    {
        caught = false;
        try
        {
            concurrent.completeActiveRequest(time);
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }
        assert(caught);
        assert(concurrent.getActiveQueueSize() == 2);
        assert(concurrent.getCompletedQueueSize() == 0);
    }
    concurrent.completeActiveRequest(1.25f);
    assert(concurrent.getActiveQueueSize() == 2);
    concurrent.completeActiveRequest(1.5f);
    assert(concurrent.getActiveQueueSize() == 1);
    assert(concurrent.getCompletedQueueSize() == 1);
    concurrent.completeActiveRequest(3.0f);
    assert(concurrent.getActiveQueueSize() == 0);
    assert(concurrent.getCompletedQueueSize() == 2);
    concurrent.completeActiveRequest(10.0f);
    assert(concurrent.getCompletedQueueSize() == 2 && "Repeated updates must not duplicate completions");

    // test completeActiveRequest() - late update ///////////////////////////////////////////
    Server overdue(9, 2.0f, 0, 2);
    accepted = overdue.submitRequest(first, 1.0f);
    assert(accepted);
    accepted = overdue.submitRequest(second, 1.0f);
    assert(accepted);
    overdue.completeActiveRequest(10.0f);
    assert(overdue.getActiveQueueSize() == 0);
    assert(overdue.getCompletedQueueSize() == 2 && "One update must complete all due Requests");

    // test operator<< - empty server and chaining ///////////////////////////////////////////
    const Server printableEmpty(10, 2.5f, 3, 2);
    std::ostringstream emptyOutput;
    std::ostream& returnedStream = (emptyOutput << printableEmpty);
    assert(&returnedStream == &emptyOutput);
    const std::string emptyText =
        "Server ID: 10 | Processing Speed: 2.5 workunits/sec per request\n"
        "Waiting (0/3) \nActive (0/2) \nCompleted (0) \n";
    assert(emptyOutput.str() == emptyText);
    returnedStream << "end";
    assert(emptyOutput.str() == emptyText + "end");
    // test completeActiveRequest() - empty server ///////////////////////////////////////////
    Server empty(11, 2.0f, 0, 1);
    empty.completeActiveRequest(0.0f);
    for (float time : invalidTimes)
    {
        caught = false;
        try
        {
            empty.completeActiveRequest(time);
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }
        assert(caught && "Completion must validate time even with no active Requests");
        assert(empty.getCompletedQueueSize() == 0);
    }

    // test operator<< - request queues and timing ///////////////////////////////////////////
    Server report(12, 2.0f, 2, 1);
    Request done(21, 1.0f, 4);
    Request waiting(22, 1.5f, 1);
    accepted = report.submitRequest(done, 1.5f);
    assert(accepted);
    accepted = report.submitRequest(waiting, 2.0f);
    assert(accepted);
    const Server& constReport = report;
    std::ostringstream beforeOutput;
    beforeOutput << constReport;
    const std::string beforeText =
        "Server ID: 12 | Processing Speed: 2 workunits/sec per request\n"
        "Waiting (1/2)  | Request ID : 22| Arrival Time 1.5| Work Unit: 1| State: PENDING\n"
        "Active (1/1)  | Request ID : 21| Arrival Time 1| Work Unit: 4| State: IN_PROGRESS"
        "| Start Time: 1.5| Waiting Time: 0.5\nCompleted (0) \n";
    assert(beforeOutput.str() == beforeText);
    std::ostringstream repeatedOutput;
    repeatedOutput << constReport;
    assert(repeatedOutput.str() == beforeText && "Output must not consume queues");
    report.completeActiveRequest(10.0f);
    std::ostringstream lateOutput;
    lateOutput << constReport;
    const std::string lateText =
        "Server ID: 12 | Processing Speed: 2 workunits/sec per request\n"
        "Waiting (1/2)  | Request ID : 22| Arrival Time 1.5| Work Unit: 1| State: PENDING\n"
        "Active (0/1) \n"
        "Completed (1)  | Request ID : 21| Arrival Time 1| Work Unit: 4| State: COMPLETED"
        "| Start Time: 1.5| Finish Time: 3.5| Waiting Time: 0.5| Response Time: 2.5\n";
    assert(lateOutput.str() == lateText && "Late update must preserve scheduled completion, not observation time");
    report.waitingQueueToActive(10.0f);
    report.completeActiveRequest(10.5f);
    assert(report.getCompletedQueueSize() == 2);
    assert(report.getWaitingQueueSize() == 0);
    assert(report.getActiveQueueSize() == 0);
    std::ostringstream finalOutput;
    finalOutput << constReport;
    assert(finalOutput.str().find("| Start Time: 10| Finish Time: 10.5| Waiting Time: 8.5| Response Time: 9") != std::string::npos);
    assert(done.getState() == Request::State::PENDING);
    assert(waiting.getState() == Request::State::PENDING);
    std::ostringstream chainedOutput;
    chainedOutput << printableEmpty << constReport;
    assert(chainedOutput.str() == emptyText + finalOutput.str());

    // test collection methods - empty queues and setup ///////////////////////////////////////////
    Server collection(13, 2.0f, 1, 2);
    Request seed(30, 0.0f, 1);
    std::vector<Request> collectedCompleted{seed};
    std::vector<Request> collectedRejected{seed};
    collection.collectCompletedRequests(collectedCompleted);
    collection.collectRejectedRequests(collectedRejected);
    assert(collectedCompleted.size() == 1 && collectedCompleted.front().getId() == 30);
    assert(collectedRejected.size() == 1 && collectedRejected.front().getId() == 30);

    Request slow(31, 1.0f, 4);
    Request fast(32, 1.0f, 1);
    Request queuedRequest(33, 1.0f, 2);
    Request refusedFirst(34, 1.0f, 1);
    Request refusedSecond(35, 1.0f, 3);
    accepted = collection.submitRequest(slow, 1.25f);
    assert(accepted);
    accepted = collection.submitRequest(fast, 1.25f);
    assert(accepted);
    accepted = collection.submitRequest(queuedRequest, 1.25f);
    assert(accepted);
    accepted = collection.submitRequest(refusedFirst, 1.25f);
    assert(!accepted);
    accepted = collection.submitRequest(refusedSecond, 1.25f);
    assert(!accepted);
    collection.completeActiveRequest(1.75f);
    assert(collection.getActiveQueueSize() == 1);
    assert(collection.getWaitingQueueSize() == 1);
    assert(collection.getCompletedQueueSize() == 1);

    // test collectCompletedRequests() ///////////////////////////////////////////
    collection.collectCompletedRequests(collectedCompleted);
    assert(collectedCompleted.size() == 2 && "Collection must append rather than replace destination contents");
    assert(collectedCompleted[0].getId() == 30);
    assert(collectedCompleted[1].getId() == 32);
    assert(collectedCompleted[1].getState() == Request::State::COMPLETED);
    assert(collectedCompleted[1].getArrivalTime() == 1.0f);
    assert(collectedCompleted[1].getWorkUnit() == 1);
    assert(collectedCompleted[1].getStartTime() == 1.25f);
    assert(collectedCompleted[1].waitingTime() == 0.25f);
    assert(collectedCompleted[1].responseTime() == 0.75f);
    assert(collection.getCompletedQueueSize() == 0);
    assert(collection.getActiveQueueSize() == 1 && "Completed collection must preserve active Requests");
    assert(collection.getWaitingQueueSize() == 1 && "Completed collection must preserve waiting Requests");
    collection.collectCompletedRequests(collectedCompleted);
    assert(collectedCompleted.size() == 2 && "Repeated collection must not duplicate Requests");

    // test collectRejectedRequests() ///////////////////////////////////////////
    collection.collectRejectedRequests(collectedRejected);
    assert(collectedRejected.size() == 3 && "Rejected collection must append every refused Request");
    assert(collectedRejected[0].getId() == 30);
    assert(collectedRejected[1].getId() == 34);
    assert(collectedRejected[2].getId() == 35 && "Rejected Requests must retain admission order");
    for (std::size_t i = 1; i < collectedRejected.size(); ++i)
    {
        assert(collectedRejected[i].getState() == Request::State::REJECTED);
        assert(collectedRejected[i].getArrivalTime() == 1.0f);
        assert(collectedRejected[i].getStartTime() == -1.0f);
        caught = false;
        try
        {
            collectedRejected[i].waitingTime();
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }
        assert(caught && "Collected rejected Requests must remain unstarted");
    }
    assert(collectedRejected[1].getWorkUnit() == 1);
    assert(collectedRejected[2].getWorkUnit() == 3);
    assert(refusedFirst.getState() == Request::State::PENDING);
    assert(refusedSecond.getState() == Request::State::PENDING);
    collection.collectRejectedRequests(collectedRejected);
    assert(collectedRejected.size() == 3 && "Rejected collection must drain the internal history");
    assert(collection.getActiveQueueSize() == 1);
    assert(collection.getWaitingQueueSize() == 1);

    // test collection methods - further processing and batches ///////////////////////////////////////////
    collection.completeActiveRequest(10.0f);
    collection.waitingQueueToActive(10.0f);
    collection.completeActiveRequest(11.0f);
    collection.collectRejectedRequests(collectedRejected);
    assert(collectedRejected.size() == 3);
    assert(collection.getCompletedQueueSize() == 2 && "Rejected collection must preserve completed history");
    collection.collectCompletedRequests(collectedCompleted);
    assert(collectedCompleted.size() == 4);
    assert(collectedCompleted[2].getId() == 31);
    assert(collectedCompleted[3].getId() == 33 && "Completed collection must preserve history order");
    assert(collectedCompleted[2].responseTime() == 2.25f && "Late collection must preserve scheduled completion");
    assert(collectedCompleted[3].waitingTime() == 9.0f);
    assert(collectedCompleted[3].responseTime() == 10.0f);
    assert(collection.getCompletedQueueSize() == 0);
    assert(collection.getWaitingQueueSize() == 0);
    assert(collection.getActiveQueueSize() == 0);
    assert(collectedCompleted[1].responseTime() == 0.75f && "Previously collected Requests must remain valid");

    // test collection methods - empty destinations and repeated collection ///////////////////////////////////////////
    Server secondBatch(14, 1.0f, 0, 1);
    std::vector<Request> emptyCompleted;
    std::vector<Request> emptyRejected;
    secondBatch.collectCompletedRequests(emptyCompleted);
    secondBatch.collectRejectedRequests(emptyRejected);
    assert(emptyCompleted.empty() && emptyRejected.empty());
    accepted = secondBatch.submitRequest(seed, 0.0f);
    assert(accepted);
    accepted = secondBatch.submitRequest(refusedFirst, 1.0f);
    assert(!accepted);
    secondBatch.collectRejectedRequests(emptyRejected);
    assert(emptyRejected.size() == 1 && emptyRejected[0].getId() == 34);
    accepted = secondBatch.submitRequest(refusedSecond, 1.0f);
    assert(!accepted);
    secondBatch.collectRejectedRequests(emptyRejected);
    assert(emptyRejected.size() == 2 && emptyRejected[1].getId() == 35);
    secondBatch.completeActiveRequest(1.0f);
    secondBatch.collectCompletedRequests(emptyCompleted);
    assert(emptyCompleted.size() == 1 && emptyCompleted[0].getId() == 30);
    assert(emptyCompleted[0].getState() == Request::State::COMPLETED);
    assert(emptyCompleted[0].responseTime() == 1.0f);
}
