#include "ComparisonReport.h"
#include <cmath>
#include <stdexcept>

static std::size_t unfinishedCount(const SimulationReport &report, std::size_t total)
{
    const auto completed = report.getCompletedRequests().size();
    const auto rejected = report.getRejectedRequests().size();
    if (completed > total || rejected > total - completed)
    {
        throw std::invalid_argument("Report results exceed the comparison traffic count.");
    }
    return total - completed - rejected;
}

static void writeSummary(std::ostream &os, const SimulationReport &report, std::size_t total)
{
    os << "Completed: " << report.getCompletedRequestsCount()
       << ", Rejected: " << report.getRejectedRequestsCount()
       << ", Unfinished: " << unfinishedCount(report, total) << '\n';
    if (report.getCompletedRequestsCount() == 0)
    {
        os << "Average waiting: N/A, Average response: N/A\n";
    }
    else
    {
        os << "Average waiting: " << report.averageWaitingTime()
           << " s, Average response: " << report.averageResponseTime() << " s\n";
    }
}

ComparisonReport::ComparisonReport(const SimulationReport &first, const SimulationReport &second, std::size_t requestCount, double timeLimit)
    : firstReport(first), secondReport(second), totalRequests(requestCount), maximumTime(timeLimit)
{
    if (!std::isfinite(timeLimit) || timeLimit <= 0.0)
    {
        throw std::invalid_argument("Comparison time limit must be finite and positive.");
    }
    unfinishedCount(firstReport, totalRequests);
    unfinishedCount(secondReport, totalRequests);
}

ComparisonReport::Outcome ComparisonReport::compare() const
{
    const auto firstCompleted = firstReport.getCompletedRequestsCount();
    const auto secondCompleted = secondReport.getCompletedRequestsCount();
    const auto firstRejected = firstReport.getRejectedRequestsCount();
    const auto secondRejected = secondReport.getRejectedRequestsCount();
    if (firstCompleted == secondCompleted && firstRejected == secondRejected)
    {
        return Outcome::EqualCounts;
    }
    if (firstCompleted >= secondCompleted && firstRejected <= secondRejected)
    {
        return Outcome::FirstBetter;
    }
    if (secondCompleted >= firstCompleted && secondRejected <= firstRejected)
    {
        return Outcome::SecondBetter;
    }
    return Outcome::TradeOff;
}

std::ostream &operator<<(std::ostream &os, const ComparisonReport &report)
{
    os << "CONFIGURATION COMPARISON\n========================\n"
       << "Requests: " << report.totalRequests << ", Time limit: " << report.maximumTime << " s\n\n";
    os << "Configuration A\n";
    writeSummary(os, report.firstReport, report.totalRequests);
    os << "\nConfiguration B\n";
    writeSummary(os, report.secondReport, report.totalRequests);
    os << "\nOutcome based on completed and rejected counts: ";
    switch (report.compare())
    {
    case ComparisonReport::Outcome::FirstBetter:
        os << "A is better";
        break;
    case ComparisonReport::Outcome::SecondBetter:
        os << "B is better";
        break;
    case ComparisonReport::Outcome::EqualCounts:
        os << "equal counts";
        break;
    case ComparisonReport::Outcome::TradeOff:
        os << "trade-off; neither dominates";
        break;
    }
    os << "\nTiming averages cover completed requests only; the completed sets may differ.\n"
          "Resource cost is not included in the comparison.\n\n"
       << "Configuration A details\n"
       << report.firstReport
       << "\nConfiguration B details\n"
       << report.secondReport;
    return os;
}
