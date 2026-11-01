#undef NDEBUG
#include <cassert>
#include <limits>
#include <sstream>
#include <stdexcept>
#include "ComparisonReport.h"
#include "Simulation.h"

static SimulationReport runScenario(const std::vector<Server>& servers, const std::vector<Request>& requests, double limit)
{
    Simulation simulation(servers, requests, limit);
    for (int step = 0; step < 30; ++step) {
        if (simulation.step() != Simulation::StepResult::EventProcessed) {
            return simulation.collectReport();
        }
    }
    throw std::runtime_error("Comparison test exceeded its step limit.");
}

void testComparisonReport()
{
    Request first(1, 0.0, 1.0);
    first.start(0.0);
    first.complete(1.0);
    Request second(2, 0.0, 1.0);
    second.start(0.5);
    second.complete(2.0);
    Request rejected(3, 0.0, 1.0);
    rejected.reject();
    const SimulationReport twoCompleted({first, second}, {});
    const SimulationReport oneCompleted({first}, {});
    const SimulationReport oneRejected({first}, {rejected});
    const SimulationReport twoAndRejected({first, second}, {rejected});
    const ComparisonReport firstBetter(twoCompleted, oneRejected, 4, 10.0);
    assert(firstBetter.compare() == ComparisonReport::Outcome::FirstBetter);
    const ComparisonReport secondBetter(oneRejected, twoCompleted, 4, 10.0);
    assert(secondBetter.compare() == ComparisonReport::Outcome::SecondBetter);
    const ComparisonReport equal(twoCompleted, twoCompleted, 4, 10.0);
    assert(equal.compare() == ComparisonReport::Outcome::EqualCounts);
    const ComparisonReport tradeOff(twoAndRejected, oneCompleted, 4, 10.0);
    assert(tradeOff.compare() == ComparisonReport::Outcome::TradeOff);
    const ComparisonReport fewerRejections(oneCompleted, oneRejected, 4, 10.0);
    assert(fewerRejections.compare() == ComparisonReport::Outcome::FirstBetter);
    const ComparisonReport moreCompletions(twoCompleted, oneCompleted, 4, 10.0);
    assert(moreCompletions.compare() == ComparisonReport::Outcome::FirstBetter);

    std::ostringstream output;
    const std::ostream& returned = (output << firstBetter);
    assert(&returned == &output);
    assert(output.str().find("Completed: 2, Rejected: 0, Unfinished: 2") != std::string::npos);
    assert(output.str().find("Completed: 1, Rejected: 1, Unfinished: 2") != std::string::npos);
    assert(output.str().find("Average waiting: 0.25 s, Average response: 1.5 s") != std::string::npos);
    assert(output.str().find("A is better") != std::string::npos);
    assert(output.str().find("Configuration A details\nSIMULATION REPORT") != std::string::npos);
    const SimulationReport empty({}, {});
    const ComparisonReport emptyComparison(empty, empty, 0, 10.0);
    std::ostringstream emptyOutput;
    emptyOutput << emptyComparison;
    assert(emptyComparison.compare() == ComparisonReport::Outcome::EqualCounts);
    assert(emptyOutput.str().find("Average waiting: N/A, Average response: N/A") != std::string::npos);
    for (const double invalidLimit : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                                      std::numeric_limits<double>::quiet_NaN()}) {
        bool caught = false;
        try {
            const ComparisonReport invalid(empty, empty, 0, invalidLimit);
        } catch (const std::invalid_argument&) {
            caught = true;
        }
        assert(caught);
    }
    for (const bool invalidFirst : {true, false}) {
        bool caught = false;
        try {
            const ComparisonReport invalid(invalidFirst ? twoCompleted : empty,
                invalidFirst ? empty : twoCompleted, 1, 10.0);
        } catch (const std::invalid_argument&) {
            caught = true;
        }
        assert(caught);
    }

    std::vector<Request> traffic;
    for (int id = 10; id < 16; ++id) traffic.emplace_back(id, 0.0, 1.0);
    const auto baseline = runScenario({Server(1, 1.0, 1, 1)}, traffic, 1.5);
    const auto expanded = runScenario({Server(1, 1.0, 1, 1), Server(2, 1.0, 1, 1)}, traffic, 1.5);
    const ComparisonReport measured(baseline, expanded, traffic.size(), 1.5);
    assert(measured.compare() == ComparisonReport::Outcome::SecondBetter);
    assert(baseline.getCompletedRequestsCount() == 1);
    assert(baseline.getRejectedRequestsCount() == 4);
    assert(expanded.getCompletedRequestsCount() == 2);
    assert(expanded.getRejectedRequestsCount() == 2);
    for (const auto& request : traffic) assert(request.getState() == Request::State::PENDING);
    const auto sameBaseline = runScenario({Server(1, 1.0, 1, 1)}, traffic, 1.5);
    const ComparisonReport repeated(baseline, sameBaseline, traffic.size(), 1.5);
    assert(repeated.compare() == ComparisonReport::Outcome::EqualCounts);
}
