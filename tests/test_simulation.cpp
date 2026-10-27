#undef NDEBUG
#include <cassert>
#include <limits>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

#include "Simulation.h"

namespace {
void assertCurrentTime(const Simulation& simulation, float expectedTime)
{
    std::ostringstream actual;
    actual << simulation;
    std::ostringstream expected;
    expected << "Current Time: " << expectedTime << "\n";
    assert(actual.str().find(expected.str()) != std::string::npos);
}
} // namespace

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


    // test step() - earliest event across servers and independent snapshots ///////////////////////////////////////////
    Server busy(2, 2.0f, 1, 2);
    bool accepted = busy.submitRequest(Request(15, 0.0f, 8), 0.0f);
    assert(accepted);
    accepted = busy.submitRequest(Request(16, 0.0f, 3), 0.0f);
    assert(accepted);
    Server faster(3, 4.0f, 0, 1);
    accepted = faster.submitRequest(Request(19, 0.0f, 3), 0.0f);
    assert(accepted);
    const Server idle(4, 1.0f, 0, 1);
    Simulation snapshots(std::vector<Server>{idle, busy, faster}, noTraffic, 20.0f);
    Simulation reordered(std::vector<Server>{faster, busy, idle}, noTraffic, 20.0f);
    faster.completeActiveRequest(0.75f);
    busy.completeActiveRequest(4.0f);
    Simulation::StepResult snapshotResult = snapshots.step();
    assert(snapshotResult == Simulation::StepResult::EventProcessed);
    assertCurrentTime(snapshots, 0.75f);
    snapshotResult = reordered.step();
    assert(snapshotResult == Simulation::StepResult::EventProcessed);
    assertCurrentTime(reordered, 0.75f);
    const SimulationReport firstSnapshot = snapshots.collectReport();
    assert(firstSnapshot.getCompletedRequestsCount() == 1);
    assert(firstSnapshot.getCompletedRequests().front().getId() == 19);
    snapshotResult = snapshots.step();
    assert(snapshotResult == Simulation::StepResult::EventProcessed);
    assertCurrentTime(snapshots, 1.5f);
    snapshotResult = snapshots.step();
    assert(snapshotResult == Simulation::StepResult::EventProcessed);
    assertCurrentTime(snapshots, 4.0f);
    snapshotResult = snapshots.step();
    assert(snapshotResult == Simulation::StepResult::Completed);
    const SimulationReport remainingSnapshot = snapshots.collectReport();
    assert(remainingSnapshot.getCompletedRequestsCount() == 2);
    Simulation finishedServers(std::vector<Server>{busy, faster}, noTraffic, 20.0f);
    snapshotResult = finishedServers.step();
    assert(snapshotResult == Simulation::StepResult::Completed);

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
    assertCurrentTime(singleStep, 0.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = singleStep.step();
    assertCurrentTime(singleStep, 1.5f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = singleStep.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - all equal-time arrivals processed together ///////////////////////////////////////////
    Simulation sameTime(std::vector<Server>{Server(21, 1.0f, 0, 2)},
        std::vector<Request>{Request(21, 1.0f, 2), Request(22, 1.0f, 1)}, 10.0f);
    result = sameTime.step();
    assertCurrentTime(sameTime, 1.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = sameTime.step();
    assertCurrentTime(sameTime, 2.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = sameTime.step();
    assertCurrentTime(sameTime, 3.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = sameTime.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - completion before arrival at the same time ///////////////////////////////////////////
    Simulation completionBeforeArrival(std::vector<Server>{Server(22, 1.0f, 0, 1)},
        std::vector<Request>{Request(23, 0.0f, 1), Request(24, 1.0f, 2)}, 10.0f);
    result = completionBeforeArrival.step();
    assertCurrentTime(completionBeforeArrival, 0.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = completionBeforeArrival.step();
    assertCurrentTime(completionBeforeArrival, 1.0f);
    assert(result == Simulation::StepResult::EventProcessed);

    // test step() - waiting requests before new arrivals ///////////////////////////////////////////
    Simulation waitingPriority(std::vector<Server>{Server(23, 1.0f, 1, 1)},
        std::vector<Request>{Request(25, 0.0f, 1), Request(26, 0.0f, 2), Request(27, 1.0f, 4)}, 20.0f);
    result = waitingPriority.step();
    assertCurrentTime(waitingPriority, 0.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = waitingPriority.step();
    assertCurrentTime(waitingPriority, 1.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = waitingPriority.step();
    assertCurrentTime(waitingPriority, 3.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = waitingPriority.step();
    assertCurrentTime(waitingPriority, 7.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = waitingPriority.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - round robin persists between steps and wraps ///////////////////////////////////////////
    Simulation roundRobin(std::vector<Server>{Server(24, 1.0f, 0, 2), Server(25, 2.0f, 0, 2)},
        std::vector<Request>{Request(28, 0.0f, 4), Request(29, 0.5f, 2), Request(30, 1.0f, 1)}, 10.0f);
    result = roundRobin.step();
    assertCurrentTime(roundRobin, 0.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = roundRobin.step();
    assertCurrentTime(roundRobin, 0.5f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = roundRobin.step();
    assertCurrentTime(roundRobin, 1.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = roundRobin.step();
    assertCurrentTime(roundRobin, 1.5f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = roundRobin.step();
    assertCurrentTime(roundRobin, 2.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = roundRobin.step();
    assertCurrentTime(roundRobin, 4.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = roundRobin.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - refusal consumes arrival without stopping simulation ///////////////////////////////////////////
    Simulation refusal(std::vector<Server>{Server(26, 1.0f, 0, 1)},
        std::vector<Request>{Request(31, 0.0f, 3), Request(32, 1.0f, 1)}, 10.0f);
    result = refusal.step();
    assertCurrentTime(refusal, 0.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = refusal.step();
    assertCurrentTime(refusal, 1.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = refusal.step();
    assertCurrentTime(refusal, 3.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = refusal.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - time limit before arrival or active completion ///////////////////////////////////////////
    Simulation lateArrival(servers, std::vector<Request>{Request(33, 5.0f, 1)}, 4.0f);
    result = lateArrival.step();
    assertCurrentTime(lateArrival, 4.0f);
    assert(result == Simulation::StepResult::TimeLimitReached);
    result = lateArrival.step();
    assertCurrentTime(lateArrival, 4.0f);
    assert(result == Simulation::StepResult::TimeLimitReached);
    Simulation lateCompletion(std::vector<Server>{Server(27, 1.0f, 0, 1)},
        std::vector<Request>{Request(34, 0.0f, 5)}, 4.0f);
    result = lateCompletion.step();
    assertCurrentTime(lateCompletion, 0.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = lateCompletion.step();
    assertCurrentTime(lateCompletion, 4.0f);
    assert(result == Simulation::StepResult::TimeLimitReached);

    // test step() - events exactly at maximum time are processed ///////////////////////////////////////////
    Simulation boundaryArrival(std::vector<Server>{Server(28, 1.0f, 0, 1)},
        std::vector<Request>{Request(35, 4.0f, 1)}, 4.0f);
    result = boundaryArrival.step();
    assertCurrentTime(boundaryArrival, 4.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = boundaryArrival.step();
    assertCurrentTime(boundaryArrival, 4.0f);
    assert(result == Simulation::StepResult::TimeLimitReached);
    Simulation boundaryCompletion(std::vector<Server>{Server(29, 1.0f, 0, 1)},
        std::vector<Request>{Request(36, 0.0f, 4)}, 4.0f);
    result = boundaryCompletion.step();
    assertCurrentTime(boundaryCompletion, 0.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = boundaryCompletion.step();
    assertCurrentTime(boundaryCompletion, 4.0f);
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
    assertCurrentTime(rejectionRotation, 1.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = rejectionRotation.step(); // Arrival at 3 must advance to free B.
    assertCurrentTime(rejectionRotation, 3.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = rejectionRotation.step();
    assertCurrentTime(rejectionRotation, 4.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = rejectionRotation.step();
    assertCurrentTime(rejectionRotation, 10.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = rejectionRotation.step();
    assert(result == Simulation::StepResult::Completed);

    // test step() - verify outcomes through reports ///////////////////////////////////////////
    result = completionBeforeArrival.step();
    assert(result == Simulation::StepResult::EventProcessed);
    const SimulationReport priorityReport = completionBeforeArrival.collectReport();
    assert(priorityReport.getCompletedRequestsCount() == 2);
    assert(priorityReport.getRejectedRequestsCount() == 0);
    assert(priorityReport.averageWaitingTime() == 0.0f);
    assert(priorityReport.averageResponseTime() == 1.5f);
    const SimulationReport waitingReport = waitingPriority.collectReport();
    assert(waitingReport.getCompletedRequestsCount() == 3);
    assert(waitingReport.getRejectedRequestsCount() == 0);
    assert(waitingReport.getCompletedRequests()[1].getId() == 26);
    assert(waitingReport.getCompletedRequests()[1].waitingTime() == 1.0f);
    assert(waitingReport.getCompletedRequests()[2].waitingTime() == 2.0f);
    const SimulationReport robinReport = roundRobin.collectReport();
    assert(robinReport.getCompletedRequestsCount() == 3);
    assert(robinReport.getRejectedRequestsCount() == 0);
    assert(robinReport.getCompletedRequests()[0].getId() == 30);
    assert(robinReport.getCompletedRequests()[0].responseTime() == 1.0f);
    assert(robinReport.getCompletedRequests()[2].getId() == 29);
    assert(robinReport.getCompletedRequests()[2].responseTime() == 1.0f);
    const SimulationReport refusedReport = refusal.collectReport();
    assert(refusedReport.getCompletedRequestsCount() == 1);
    assert(refusedReport.getRejectedRequestsCount() == 1);
    assert(refusedReport.getRejectedRequests().front().getId() == 32);
    const SimulationReport lateArrivalReport = lateArrival.collectReport();
    assert(lateArrivalReport.getCompletedRequestsCount() == 0);
    assert(lateArrivalReport.getRejectedRequestsCount() == 0);
    const SimulationReport lateCompletionReport = lateCompletion.collectReport();
    assert(lateCompletionReport.getCompletedRequestsCount() == 0);
    const SimulationReport boundaryReport = boundaryCompletion.collectReport();
    assert(boundaryReport.getCompletedRequestsCount() == 1);
    assert(boundaryReport.averageResponseTime() == 4.0f);

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
                       << outputServers[0] << "\n" << outputServers[1] << "\n";
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
                         << "  " << outputTraffic[0] << "\n  " << outputTraffic[1] << "\n" << "\nServers:\n"
                         << outputServers[0] << "\n" << outputServers[1] << "\n";
    assert(initialState.str() == expectedInitialState.str() && "Output must preserve incoming and server order");
    result = printableSimulation.step();
    assertCurrentTime(printableSimulation, 1.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    std::vector<Server> expectedOutputServers = outputServers;
    accepted = expectedOutputServers[0].submitRequest(outputTraffic[0], 1.0f);
    assert(accepted);
    std::ostringstream steppedState;
    steppedState << printableView;
    std::ostringstream expectedSteppedState;
    expectedSteppedState << "Simulation State:\nCurrent Time: 1\nMaximum Time: 12\n"
                         << "Next Arrival Index: 1\nNext Server Index: 1\nIncoming Requests:\n"
                         << "  " << outputTraffic[0] << "\n  " << outputTraffic[1] << "\n" << "\nServers:\n"
                         << expectedOutputServers[0] << "\n" << expectedOutputServers[1] << "\n";
    assert(steppedState.str() == expectedSteppedState.str());
    std::ostringstream repeatedState;
    repeatedState << printableView;
    assert(repeatedState.str() == steppedState.str() && "Output must not advance simulation or consume queues");
    result = printableSimulation.step();
    assertCurrentTime(printableSimulation, 2.0f);
    assert(result == Simulation::StepResult::EventProcessed);
    result = printableSimulation.step();
    assertCurrentTime(printableSimulation, 3.0f);
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
    assertCurrentTime(limitedOutput, 2.5f);
    assert(result == Simulation::StepResult::TimeLimitReached);
    std::ostringstream limitState;
    limitState << limitedOutput;
    assert(limitState.str().find("Current Time: 2.5\nMaximum Time: 2.5\n") != std::string::npos);
    assert(limitState.str().find("Next Arrival Index: 0\nNext Server Index: 0\n") != std::string::npos);

    // test fractional workUnit - full simulation and report ///////////////////////////////////////////
    Simulation fractionalSimulation(std::vector<Server>{Server(100, 2.0f, 1, 1)},
        std::vector<Request>{Request(100, 0.0f, 1.5f), Request(101, 0.0f, 1.5f)}, 10.0f);
    result = fractionalSimulation.step();
    assert(result == Simulation::StepResult::EventProcessed);
    result = fractionalSimulation.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assertCurrentTime(fractionalSimulation, 0.75f);
    result = fractionalSimulation.step();
    assert(result == Simulation::StepResult::EventProcessed);
    assertCurrentTime(fractionalSimulation, 1.5f);
    result = fractionalSimulation.step();
    assert(result == Simulation::StepResult::Completed);
    const SimulationReport fractionalSimulationReport = fractionalSimulation.collectReport();
    assert(fractionalSimulationReport.getCompletedRequestsCount() == 2);
    assert(fractionalSimulationReport.getRejectedRequestsCount() == 0);
    assert(fractionalSimulationReport.averageWaitingTime() == 0.375f);
    assert(fractionalSimulationReport.averageResponseTime() == 1.125f);
    assert(fractionalSimulationReport.getCompletedRequests()[0].getWorkUnit() == 1.5f);
    assert(fractionalSimulationReport.getCompletedRequests()[1].getWorkUnit() == 1.5f);
}
