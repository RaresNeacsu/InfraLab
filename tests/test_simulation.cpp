#undef NDEBUG
#include <cassert>
#include <limits>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

#include "Simulation.h"

void testSimulation()
{
    const std::vector<Server> servers{Server(1, 2.0f, 2, 1)};
    const std::vector<Server> noServers;
    const std::vector<Request> noTraffic;
    const std::vector<Request> traffic{Request(1, 1.0f, 2)};
    bool caught = false;

    // test constructor - empty servers ///////////////////////////////////////////
    caught = false;
    try
    {
        const Simulation simulation(noServers, noTraffic, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected empty servers to throw with empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(noServers, traffic, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected empty servers to throw with non-empty traffic");

    // test constructor - valid traffic ///////////////////////////////////////////
    caught = false;
    try
    {
        const Simulation simulation(servers, noTraffic, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected valid servers and time to accept empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(servers, traffic, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected one pending Request to be accepted");

    // test constructor - non-pending requests ///////////////////////////////////////////
    Request started(2, 1.0f, 1);
    started.start(1.0f);
    Request completed = started;
    completed.complete(2.0f);
    Request rejected(3, 1.0f, 1);
    rejected.reject();

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{started}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected started Request alone to throw");

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{started, traffic.front()}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected started Request at the first position to throw");

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{traffic.front(), started}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected started Request at a later position to throw");

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{completed}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected completed Request alone to throw");

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{completed, traffic.front()}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected completed Request at the first position to throw");

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{traffic.front(), completed}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected completed Request at a later position to throw");

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{rejected}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected rejected Request alone to throw");

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{rejected, traffic.front()}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected rejected Request at the first position to throw");

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{traffic.front(), rejected}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected rejected Request at a later position to throw");

    // test constructor - arrival ordering ///////////////////////////////////////////
    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{Request(4, 2.0f, 1), Request(5, 1.0f, 1)}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected decreasing arrivals to throw");

    caught = false;
    try
    {
        const Simulation simulation(servers, std::vector<Request>{Request(4, 0.0f, 1), Request(5, 2.0f, 1), Request(6, 1.0f, 1)}, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected a later decrease in arrival time to throw");

    const std::vector<Request> orderedTraffic{
        Request(4, 0.0f, 1), Request(5, 1.0f, 1), Request(6, 1.0f, 1), Request(7, 2.0f, 1)
    };
    caught = false;
    try
    {
        const Simulation simulation(servers, orderedTraffic, 10.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected ordered traffic with equal arrivals to be accepted");

    // test constructor - invalid maximum time with non-empty traffic ///////////////////////////////////////////
    caught = false;
    try
    {
        const Simulation simulation(servers, traffic, 0.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected zero maximum time to throw with non-empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(servers, traffic, -1.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected negative maximum time to throw with non-empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(servers, traffic, std::numeric_limits<float>::quiet_NaN());
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected NaN maximum time to throw with non-empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(servers, traffic, std::numeric_limits<float>::infinity());
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected positive infinite maximum time to throw with non-empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(servers, traffic, -std::numeric_limits<float>::infinity());
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected negative infinite maximum time to throw with non-empty traffic");

    // test constructor - invalid maximum time with empty traffic ///////////////////////////////////////////
    caught = false;
    try
    {
        const Simulation simulation(servers, noTraffic, 0.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected zero maximum time to throw with empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(servers, noTraffic, -1.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected negative maximum time to throw with empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(servers, noTraffic, std::numeric_limits<float>::quiet_NaN());
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected NaN maximum time to throw with empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(servers, noTraffic, std::numeric_limits<float>::infinity());
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected positive infinite maximum time to throw with empty traffic");

    caught = false;
    try
    {
        const Simulation simulation(servers, noTraffic, -std::numeric_limits<float>::infinity());
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected negative infinite maximum time to throw with empty traffic");


    // test nextEventTime() - no events ///////////////////////////////////////////
    const Simulation noEvents(servers, noTraffic, 20.0f);
    assert(noEvents.nextEventTime() == std::numeric_limits<float>::infinity() &&
           "Idle servers without incoming traffic must have no next event");

    // test nextEventTime() - arrivals only ///////////////////////////////////////////
    const std::vector<Request> arrivals{Request(10, 2.5f, 1), Request(11, 4.0f, 1)};
    const Simulation arrivalsOnly(servers, arrivals, 20.0f);
    assert(arrivalsOnly.nextEventTime() == 2.5f && "Expected the next arrival time");
    assert(arrivalsOnly.nextEventTime() == 2.5f && "Query must not advance the arrival index");
    const Simulation zeroArrival(servers, std::vector<Request>{Request(12, 0.0f, 1)}, 20.0f);
    assert(zeroArrival.nextEventTime() == 0.0f && "An arrival at zero is a valid next event");
    const Simulation equalArrivals(servers,
        std::vector<Request>{Request(13, 2.5f, 1), Request(14, 2.5f, 1)}, 20.0f);
    assert(equalArrivals.nextEventTime() == 2.5f);

    // test nextEventTime() - completions only ///////////////////////////////////////////
    Server busy(2, 2.0f, 1, 2);
    bool accepted = busy.submitRequest(Request(15, 0.0f, 8), 0.0f);
    assert(accepted); // finish 4
    accepted = busy.submitRequest(Request(16, 0.0f, 3), 0.0f);
    assert(accepted); // finish 1.5, despite being inserted second
    const Simulation completionOnly(std::vector<Server>{busy}, noTraffic, 20.0f);
    assert(completionOnly.nextEventTime() == 1.5f && "Expected the earliest active completion");
    assert(completionOnly.nextEventTime() == 1.5f && "Query must not finish or remove active Requests");
    assert(busy.getActiveQueueSize() == 2);
    assert(busy.getWaitingQueueSize() == 0);
    assert(busy.getCompletedQueueSize() == 0);

    // test nextEventTime() - arrival versus completion ///////////////////////////////////////////
    const Simulation arrivalFirst(std::vector<Server>{busy},
        std::vector<Request>{Request(17, 0.5f, 1)}, 20.0f);
    assert(arrivalFirst.nextEventTime() == 0.5f && "Arrival before completion must be selected");
    const Simulation completionFirst(std::vector<Server>{busy}, arrivals, 20.0f);
    assert(completionFirst.nextEventTime() == 1.5f && "Completion before arrival must be selected");
    const Simulation tiedEvents(std::vector<Server>{busy},
        std::vector<Request>{Request(18, 1.5f, 1)}, 20.0f);
    assert(tiedEvents.nextEventTime() == 1.5f && "Equal event times must preserve their shared timestamp");
    // This query selects a timestamp; completion-first handling at a tie belongs to the event loop.

    // test nextEventTime() - earliest completion across multiple servers ///////////////////////////////////////////
    Server faster(3, 4.0f, 0, 1);
    accepted = faster.submitRequest(Request(19, 0.0f, 3), 0.0f);
    assert(accepted); // finish 0.75
    const Server idle(4, 1.0f, 0, 1);
    const Simulation severalServers(std::vector<Server>{idle, busy, faster}, arrivals, 20.0f);
    assert(severalServers.nextEventTime() == 0.75f && "Every server must contribute to the minimum");
    const Simulation reorderedServers(std::vector<Server>{faster, busy, idle}, arrivals, 20.0f);
    assert(reorderedServers.nextEventTime() == 0.75f && "Server order must not affect event selection");

    // test nextEventTime() - simulation owns its server snapshot ///////////////////////////////////////////
    faster.completeActiveRequest(0.75f);
    busy.completeActiveRequest(4.0f);
    assert(severalServers.nextEventTime() == 0.75f && "External server changes must not change simulation data");
    assert(completionOnly.nextEventTime() == 1.5f);
    const Simulation finishedServers(std::vector<Server>{busy, faster}, noTraffic, 20.0f);
    assert(finishedServers.nextEventTime() == std::numeric_limits<float>::infinity() &&
           "Completed history must not count as an active event");

    // test step() - no events and repeated completion ///////////////////////////////////////////
    Simulation idleSimulation(servers, noTraffic, 10.0f);
    Simulation::StepResult result = idleSimulation.step();
    assert(result == Simulation::StepResult::Completed);
    result = idleSimulation.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - arrival at zero and fractional completion ///////////////////////////////////////////
    Simulation singleStep(std::vector<Server>{Server(20, 2.0f, 0, 1)},
        std::vector<Request>{Request(20, 0.0f, 3)}, 10.0f);
    result = singleStep.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(singleStep.nextEventTime() == 1.5f && "Arrival must be consumed and its completion scheduled");
    result = singleStep.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(singleStep.nextEventTime() == std::numeric_limits<float>::infinity());
    result = singleStep.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - all equal-time arrivals processed together ///////////////////////////////////////////
    Simulation sameTime(std::vector<Server>{Server(21, 1.0f, 0, 2)},
        std::vector<Request>{Request(21, 1.0f, 2), Request(22, 1.0f, 1)}, 10.0f);
    result = sameTime.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(sameTime.nextEventTime() == 2.0f && "Both arrivals must be processed in one step");
    result = sameTime.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(sameTime.nextEventTime() == 3.0f);
    result = sameTime.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = sameTime.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - completion before arrival at the same time ///////////////////////////////////////////
    Simulation completionBeforeArrival(std::vector<Server>{Server(22, 1.0f, 0, 1)},
        std::vector<Request>{Request(23, 0.0f, 1), Request(24, 1.0f, 2)}, 10.0f);
    result = completionBeforeArrival.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(completionBeforeArrival.nextEventTime() == 1.0f);
    result = completionBeforeArrival.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(completionBeforeArrival.nextEventTime() == 3.0f &&
           "Completion must free the slot before the simultaneous arrival is admitted");

    // test step() - waiting requests before new arrivals ///////////////////////////////////////////
    Simulation waitingPriority(std::vector<Server>{Server(23, 1.0f, 1, 1)},
        std::vector<Request>{Request(25, 0.0f, 1), Request(26, 0.0f, 2), Request(27, 1.0f, 4)}, 20.0f);
    result = waitingPriority.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = waitingPriority.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(waitingPriority.nextEventTime() == 3.0f && "Older waiting Request must start before new arrival");
    result = waitingPriority.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(waitingPriority.nextEventTime() == 7.0f && "Completion-only step must also start waiting Requests");
    result = waitingPriority.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = waitingPriority.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - round robin persists between steps and wraps ///////////////////////////////////////////
    Simulation roundRobin(std::vector<Server>{Server(24, 1.0f, 0, 2), Server(25, 2.0f, 0, 2)},
        std::vector<Request>{Request(28, 0.0f, 4), Request(29, 0.5f, 2), Request(30, 1.0f, 1)}, 10.0f);
    result = roundRobin.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(roundRobin.nextEventTime() == 0.5f);
    result = roundRobin.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(roundRobin.nextEventTime() == 1.0f);
    result = roundRobin.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(roundRobin.nextEventTime() == 1.5f && "Second arrival must use the faster second server");
    result = roundRobin.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(roundRobin.nextEventTime() == 2.0f && "Third arrival must wrap back to the first server");
    result = roundRobin.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(roundRobin.nextEventTime() == 4.0f);
    result = roundRobin.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = roundRobin.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - refusal consumes arrival without stopping simulation ///////////////////////////////////////////
    Simulation refusal(std::vector<Server>{Server(26, 1.0f, 0, 1)},
        std::vector<Request>{Request(31, 0.0f, 3), Request(32, 1.0f, 1)}, 10.0f);
    result = refusal.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = refusal.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(refusal.nextEventTime() == 3.0f && "Rejected arrival must not remain pending or become active");
    result = refusal.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = refusal.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - time limit before arrival or active completion ///////////////////////////////////////////
    Simulation lateArrival(servers, std::vector<Request>{Request(33, 5.0f, 1)}, 4.0f);
    result = lateArrival.step();
    assert(result == Simulation::StepResult::TimeLimitReached);
    assert(lateArrival.nextEventTime() == 5.0f && "Time limit must not consume the future arrival");
    result = lateArrival.step();
    assert(result == Simulation::StepResult::TimeLimitReached);
    Simulation lateCompletion(std::vector<Server>{Server(27, 1.0f, 0, 1)},
        std::vector<Request>{Request(34, 0.0f, 5)}, 4.0f);
    result = lateCompletion.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = lateCompletion.step();
    assert(result == Simulation::StepResult::TimeLimitReached);
    assert(lateCompletion.nextEventTime() == 5.0f && "Time limit must not finalize a later completion");

    // test step() - events exactly at maximum time are processed ///////////////////////////////////////////
    Simulation boundaryArrival(std::vector<Server>{Server(28, 1.0f, 0, 1)},
        std::vector<Request>{Request(35, 4.0f, 1)}, 4.0f);
    result = boundaryArrival.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(boundaryArrival.nextEventTime() == 5.0f);
    result = boundaryArrival.step();
    assert(result == Simulation::StepResult::TimeLimitReached);
    Simulation boundaryCompletion(std::vector<Server>{Server(29, 1.0f, 0, 1)},
        std::vector<Request>{Request(36, 0.0f, 4)}, 4.0f);
    result = boundaryCompletion.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = boundaryCompletion.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = boundaryCompletion.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - no fallback and round robin after rejection ///////////////////////////////////////////
    Server occupiedA(40, 1.0f, 0, 1);
    const Server freeB(41, 1.0f, 0, 1);
    accepted = occupiedA.submitRequest(Request(40, 0.0f, 10), 0.0f);
    assert(accepted); // A remains busy until 10; B starts idle.
    Simulation rejectionRotation(std::vector<Server>{occupiedA, freeB},
        std::vector<Request>{Request(41, 1.0f, 1), Request(42, 3.0f, 1)}, 20.0f);
    result = rejectionRotation.step(); // Arrival at 1 is assigned to full A.
    assert(result == Simulation::StepResult::EventProcessed);
    assert(rejectionRotation.nextEventTime() == 3.0f &&
           "Rejected arrival must not be retried on free B, which would finish it at 2");
    result = rejectionRotation.step(); // Arrival at 3 must advance to free B.
    assert(result == Simulation::StepResult::EventProcessed);
    assert(rejectionRotation.nextEventTime() == 4.0f &&
           "Rejection on A must advance round robin so the following arrival runs on B");
    result = rejectionRotation.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(rejectionRotation.nextEventTime() == 10.0f);
    result = rejectionRotation.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(rejectionRotation.nextEventTime() == std::numeric_limits<float>::infinity());
    result = rejectionRotation.step();
    assert(result == Simulation::StepResult::Completed);

    // test collectReport() - original scenario ///////////////////////////////////////////
    const std::vector<Server> originalServers{
        Server(50, 2.0f, 2, 1), Server(51, 2.0f, 2, 1)
    };
    const std::vector<Request> originalTraffic{
        Request(50, 0.0f, 6), Request(51, 0.0f, 6), Request(52, 0.0f, 6)
    };
    Simulation originalScenario(originalServers, originalTraffic, 20.0f);
    result = originalScenario.step();
    int originalSteps = 1;
    while (result == Simulation::StepResult::EventProcessed && originalSteps < 10)
    {
        result = originalScenario.step();
        ++originalSteps;
    }
    assert(result == Simulation::StepResult::Completed && "Original scenario must finish within the step bound");
    const SimulationReport originalReport = originalScenario.collectReport();
    assert(originalReport.getCompletedRequestsCount() == 3);
    assert(originalReport.getRejectedRequestsCount() == 0);
    assert(originalReport.averageWaitingTime() == 1.0f);
    assert(originalReport.averageResponseTime() == 4.0f);
    assert(originalReport.getCompletedRequests().size() == 3);
    assert(originalReport.getRejectedRequests().empty());
    // Server A completes requests 50 and 52; B completes request 51.
    // Report order is server collection order, not global completion order.
    const std::vector<Request>& originalCompleted = originalReport.getCompletedRequests();
    assert(originalCompleted[0].getId() == 50);
    assert(originalCompleted[0].waitingTime() == 0.0f);
    assert(originalCompleted[0].responseTime() == 3.0f);
    assert(originalCompleted[1].getId() == 52);
    assert(originalCompleted[1].waitingTime() == 3.0f);
    assert(originalCompleted[1].responseTime() == 6.0f);
    assert(originalCompleted[2].getId() == 51);
    assert(originalCompleted[2].waitingTime() == 0.0f);
    assert(originalCompleted[2].responseTime() == 3.0f);
    const SimulationReport drainedReport = originalScenario.collectReport();
    assert(drainedReport.getCompletedRequestsCount() == 0);
    assert(drainedReport.getRejectedRequestsCount() == 0);
    assert(drainedReport.getCompletedRequests().empty());
    assert(drainedReport.getRejectedRequests().empty());
    assert(originalReport.getCompletedRequestsCount() == 3 && "Collecting again must not change the previous report");
    assert(originalReport.averageWaitingTime() == 1.0f);
    assert(originalReport.averageResponseTime() == 4.0f);

    // test collectReport() - completed and rejected results across servers ///////////////////////////////////////////
    const SimulationReport rotationReport = rejectionRotation.collectReport();
    assert(rotationReport.getCompletedRequestsCount() == 2);
    assert(rotationReport.getRejectedRequestsCount() == 1);
    assert(rotationReport.getRejectedRequests().front().getId() == 41);
    assert(rotationReport.getRejectedRequests().front().getState() == Request::State::REJECTED);
    const SimulationReport drainedRotation = rejectionRotation.collectReport();
    assert(drainedRotation.getCompletedRequestsCount() == 0);
    assert(drainedRotation.getRejectedRequestsCount() == 0);

    // test operator<< - empty traffic and const Simulation ///////////////////////////////////////////
    const std::vector<Server> outputServers{Server(60, 2.0f, 1, 1), Server(61, 1.0f, 1, 1)};
    const Simulation outputEmpty(outputServers, noTraffic, 12.0f);
    std::ostringstream emptyState;
    std::ostream& returnedStream = (emptyState << outputEmpty);
    assert(&returnedStream == &emptyState && "Operator must return the supplied stream");
    std::ostringstream expectedEmptyState;
    expectedEmptyState << "Simulation State:\nCurrent Time: 0\nMaximum Time: 12\n"
                       << "Next Arrival Index: 0\nNext Server Index: 0\n"
                       << "Incoming Requests:\n\nServers:\n"
                       << outputServers[0] << outputServers[1];
    assert(emptyState.str() == expectedEmptyState.str());
    returnedStream << "end";
    assert(emptyState.str() == expectedEmptyState.str() + "end");

    // test operator<< - initial and stepped state with multiple servers ///////////////////////////////////////////
    const std::vector<Request> outputTraffic{Request(60, 1.0f, 4), Request(61, 2.0f, 1)};
    Simulation printableSimulation(outputServers, outputTraffic, 12.0f);
    const Simulation& printableView = printableSimulation;
    std::ostringstream initialState;
    initialState << printableView;
    std::ostringstream expectedInitialState;
    expectedInitialState << "Simulation State:\nCurrent Time: 0\nMaximum Time: 12\n"
                         << "Next Arrival Index: 0\nNext Server Index: 0\nIncoming Requests:\n"
                         << outputTraffic[0] << outputTraffic[1] << "\nServers:\n"
                         << outputServers[0] << outputServers[1];
    assert(initialState.str() == expectedInitialState.str() && "Output must preserve incoming and server order");
    assert(printableSimulation.nextEventTime() == 1.0f);
    result = printableSimulation.step();
    assert(result == Simulation::StepResult::EventProcessed);
    std::vector<Server> expectedOutputServers = outputServers;
    accepted = expectedOutputServers[0].submitRequest(outputTraffic[0], 1.0f);
    assert(accepted);
    std::ostringstream steppedState;
    steppedState << printableView;
    std::ostringstream expectedSteppedState;
    expectedSteppedState << "Simulation State:\nCurrent Time: 1\nMaximum Time: 12\n"
                         << "Next Arrival Index: 1\nNext Server Index: 1\nIncoming Requests:\n"
                         << outputTraffic[0] << outputTraffic[1] << "\nServers:\n"
                         << expectedOutputServers[0] << expectedOutputServers[1];
    assert(steppedState.str() == expectedSteppedState.str());
    std::ostringstream repeatedState;
    repeatedState << printableView;
    assert(repeatedState.str() == steppedState.str() && "Output must not advance simulation or consume queues");
    assert(printableSimulation.nextEventTime() == 2.0f);
    result = printableSimulation.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assert(printableSimulation.nextEventTime() == 3.0f);
    result = printableSimulation.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = printableSimulation.step();
    assert(result == Simulation::StepResult::Completed);
    std::ostringstream completedState;
    completedState << printableView;
    assert(completedState.str().find("Current Time: 3\n") != std::string::npos);
    assert(completedState.str().find("Next Arrival Index: 2\nNext Server Index: 0\n") != std::string::npos);
    const SimulationReport printedReport = printableSimulation.collectReport();
    assert(printedReport.getCompletedRequestsCount() == 2 && "Printing must preserve results for collection");
    assert(printedReport.getRejectedRequestsCount() == 0);
    assert(printedReport.averageWaitingTime() == 0.0f);
    assert(printedReport.averageResponseTime() == 1.5f);
    std::ostringstream chainedState;
    chainedState << outputEmpty << printableView;
    std::ostringstream drainedState;
    drainedState << printableView;
    assert(chainedState.str() == expectedEmptyState.str() + drainedState.str());

    // test operator<< - time limit state ///////////////////////////////////////////
    Simulation limitedOutput(outputServers, std::vector<Request>{Request(62, 5.0f, 1)}, 2.5f);
    result = limitedOutput.step();
    assert(result == Simulation::StepResult::TimeLimitReached);
    std::ostringstream limitState;
    limitState << limitedOutput;
    assert(limitState.str().find("Current Time: 2.5\nMaximum Time: 2.5\n") != std::string::npos);
    assert(limitState.str().find("Next Arrival Index: 0\nNext Server Index: 0\n") != std::string::npos);
    assert(limitedOutput.nextEventTime() == 5.0f && "Printing a limited simulation must not consume future arrivals");
}
