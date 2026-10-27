#undef NDEBUG
#include <cassert>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "SimulationReport.h"

void testSimulationReport()
{
    // test constructor and getters ///////////////////////////////////////////
    Request pending(1, 0.0f, 1);
    Request started(2, 0.0f, 1);
    started.start(0.0f);
    Request first(3, 1.0f, 4);
    first.start(1.5f);
    first.complete(3.5f); // waiting 0.5, response 2.5
    Request second(4, 2.0f, 1);
    second.start(2.0f);
    second.complete(2.25f); // waiting 0, response 0.25
    Request third(5, 1.0f, 2);
    third.start(2.5f);
    third.complete(4.0f); // waiting 1.5, response 3
    Request rejected(6, 1.0f, 1);
    rejected.reject();
    Request otherRejected(7, 2.0f, 2);
    otherRejected.reject();

    const std::vector<Request> noRequests;
    const std::vector<Request> completedRequests{first, second, third};
    const std::vector<Request> rejectedRequests{rejected, otherRejected};
    const SimulationReport empty(noRequests, noRequests);
    const SimulationReport completedOnly(completedRequests, noRequests);
    const SimulationReport rejectedOnly(noRequests, rejectedRequests);
    const SimulationReport mixed(completedRequests, rejectedRequests);
    assert(empty.getCompletedRequestsCount() == 0);
    assert(empty.getRejectedRequestsCount() == 0);
    assert(completedOnly.getCompletedRequestsCount() == 3);
    assert(completedOnly.getRejectedRequestsCount() == 0);
    assert(rejectedOnly.getCompletedRequestsCount() == 0);
    assert(rejectedOnly.getRejectedRequestsCount() == 2);
    assert(mixed.getCompletedRequestsCount() == 3);
    assert(mixed.getRejectedRequestsCount() == 2);

    // test constructor - invalid request states ///////////////////////////////////////////
    bool caught = false;
    caught = false;
    try
    {
        const SimulationReport invalid(std::vector<Request>{first, pending}, rejectedRequests);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected pending Request in completed input to throw");

    caught = false;
    try
    {
        const SimulationReport invalid(std::vector<Request>{first, started}, rejectedRequests);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected started Request in completed input to throw");

    caught = false;
    try
    {
        const SimulationReport invalid(std::vector<Request>{first, rejected}, rejectedRequests);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected rejected Request in completed input to throw");

    caught = false;
    try
    {
        const SimulationReport invalid(completedRequests, std::vector<Request>{rejected, pending});
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected pending Request in rejected input to throw");

    caught = false;
    try
    {
        const SimulationReport invalid(completedRequests, std::vector<Request>{rejected, started});
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected started Request in rejected input to throw");

    caught = false;
    try
    {
        const SimulationReport invalid(completedRequests, std::vector<Request>{rejected, first});
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected completed Request in rejected input to throw");

    // test averageWaitingTime() ///////////////////////////////////////////
    const SimulationReport single(std::vector<Request>{first}, noRequests);
    assert(single.averageWaitingTime() == 0.5f);
    const SimulationReport zeroWaiting(std::vector<Request>{second}, noRequests);
    assert(zeroWaiting.averageWaitingTime() == 0.0f);
    const float tolerance = 0.00001f;
    assert(std::fabs(mixed.averageWaitingTime() - 2.0f / 3.0f) < tolerance);
    assert(std::fabs(completedOnly.averageWaitingTime() - mixed.averageWaitingTime()) < tolerance &&
           "Rejected Requests must not affect waiting time averages");
    assert(std::fabs(mixed.averageWaitingTime() - 2.0f / 3.0f) < tolerance &&
           "Repeated calls must preserve the result");

    // test averageResponseTime() ///////////////////////////////////////////
    assert(single.averageResponseTime() == 2.5f);
    assert(zeroWaiting.averageResponseTime() == 0.25f);
    // (2.5 + 0.25 + 3) / 3 = 5.75/3.
    assert(std::fabs(mixed.averageResponseTime() - 5.75f / 3.0f) < tolerance);
    assert(std::fabs(completedOnly.averageResponseTime() - mixed.averageResponseTime()) < tolerance &&
           "Rejected Requests must not affect response time averages");
    assert(std::fabs(mixed.averageResponseTime() - 5.75f / 3.0f) < tolerance);

    // test average methods - no completed requests ///////////////////////////////////////////
    caught = false;
    try
    {
        empty.averageWaitingTime();
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    assert(caught && "Expected waiting average of empty report to throw");

    caught = false;
    try
    {
        empty.averageResponseTime();
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    assert(caught && "Expected response average of empty report to throw");

    assert(empty.getCompletedRequestsCount() == 0);

    caught = false;
    try
    {
        rejectedOnly.averageWaitingTime();
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    assert(caught && "Expected waiting average of rejected-only report to throw");

    caught = false;
    try
    {
        rejectedOnly.averageResponseTime();
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    assert(caught && "Expected response average of rejected-only report to throw");

    assert(rejectedOnly.getCompletedRequestsCount() == 0);

    // test constructor - independent ownership of input data ///////////////////////////////////////////
    std::vector<Request> sourceCompleted{first};
    std::vector<Request> sourceRejected{rejected};
    const SimulationReport snapshot(sourceCompleted, sourceRejected);
    sourceCompleted[0] = second;
    sourceRejected.clear();
    sourceCompleted.clear();
    assert(snapshot.getCompletedRequestsCount() == 1);
    assert(snapshot.getRejectedRequestsCount() == 1);
    assert(snapshot.averageWaitingTime() == 0.5f);
    assert(snapshot.averageResponseTime() == 2.5f);
    const SimulationReport fromTemporaries(std::vector<Request>{second}, std::vector<Request>{rejected});
    assert(fromTemporaries.averageResponseTime() == 0.25f);

    // test operator<< - empty and rejected-only reports ///////////////////////////////////////////
    std::ostringstream emptyOutput;
    std::ostream& returnedStream = (emptyOutput << empty);
    assert(&returnedStream == &emptyOutput);
    const std::string emptyText =
        "SIMULATION REPORT\n=================\n\nCompleted Requests: 0\n\nRejected Requests: 0\n\n"
        "No completed requests to calculate average times.\n";
    assert(emptyOutput.str() == emptyText);
    returnedStream << "end";
    assert(emptyOutput.str() == emptyText + "end");
    std::ostringstream rejectedOutput;
    rejectedOutput << rejectedOnly;
    std::ostringstream expectedRejected;
    expectedRejected << "SIMULATION REPORT\n=================\n\nCompleted Requests: 0\n\nRejected Requests: 2\n"
                     << "  " << rejected << "\n  " << otherRejected << "\n\n"
                     << "No completed requests to calculate average times.\n";
    assert(rejectedOutput.str() == expectedRejected.str());

    // test operator<< - completed and mixed reports ///////////////////////////////////////////
    const SimulationReport printable(std::vector<Request>{first, second}, rejectedRequests);
    std::ostringstream output;
    output << printable;
    std::ostringstream expected;
    expected << "SIMULATION REPORT\n=================\n\nCompleted Requests: 2\n"
             << "  " << first << "\n  " << second << "\n\nRejected Requests: 2\n"
             << "  " << rejected << "\n  " << otherRejected << "\n\n"
             << "Average Waiting Time: 0.25 s\nAverage Response Time: 1.375 s\n";
    assert(output.str() == expected.str() && "Output must list each Request in input order with correct averages");
    const SimulationReport printableCompleted(std::vector<Request>{first}, noRequests);
    std::ostringstream completedOutput;
    completedOutput << printableCompleted;
    std::ostringstream expectedCompleted;
    expectedCompleted << "SIMULATION REPORT\n=================\n\nCompleted Requests: 1\n  " << first
                      << "\n\nRejected Requests: 0\n\nAverage Waiting Time: 0.5 s\nAverage Response Time: 2.5 s\n";
    assert(completedOutput.str() == expectedCompleted.str());
    std::ostringstream repeatedOutput;
    repeatedOutput << printable;
    assert(repeatedOutput.str() == output.str() && "Displaying the report must not consume its data");
    assert(printable.getCompletedRequestsCount() == 2);
    assert(printable.getRejectedRequestsCount() == 2);
    assert(printable.averageWaitingTime() == 0.25f);
    assert(printable.averageResponseTime() == 1.375f);
    std::ostringstream chainedOutput;
    chainedOutput << empty << printable;
    assert(chainedOutput.str() == emptyText + output.str());

    // test getCompletedRequests() and getRejectedRequests() ///////////////////////////////////////////
    assert(empty.getCompletedRequests().empty());
    assert(empty.getRejectedRequests().empty());
    const std::vector<Request>& completedView = mixed.getCompletedRequests();
    const std::vector<Request>& rejectedView = mixed.getRejectedRequests();
    assert(completedView.size() == 3);
    assert(completedView[0].getId() == first.getId());
    assert(completedView[1].getId() == second.getId());
    assert(completedView[2].getId() == third.getId());
    assert(completedView[0].getState() == Request::State::COMPLETED);
    assert(completedView[0].waitingTime() == 0.5f);
    assert(completedView[0].responseTime() == 2.5f);
    assert(rejectedView.size() == 2);
    assert(rejectedView[0].getId() == rejected.getId());
    assert(rejectedView[1].getId() == otherRejected.getId());
    assert(rejectedView[0].getState() == Request::State::REJECTED);
    assert(rejectedView[1].getState() == Request::State::REJECTED);
    assert(snapshot.getCompletedRequests()[0].getId() == first.getId());
    assert(snapshot.getRejectedRequests()[0].getId() == rejected.getId());

    // test fractional workUnit - report preserves request data ///////////////////////////////////////////
    Request fractionalCompleted(100, 0.0f, 1.5f);
    fractionalCompleted.start(0.25f);
    fractionalCompleted.complete(1.0f);
    Request fractionalRejected(101, 0.0f, 0.5f);
    fractionalRejected.reject();
    const SimulationReport fractionalReport(
        std::vector<Request>{fractionalCompleted}, std::vector<Request>{fractionalRejected});
    assert(fractionalReport.getCompletedRequests()[0].getWorkUnit() == 1.5f);
    assert(fractionalReport.getRejectedRequests()[0].getWorkUnit() == 0.5f);
    assert(fractionalReport.averageWaitingTime() == 0.25f);
    assert(fractionalReport.averageResponseTime() == 1.0f);
}
