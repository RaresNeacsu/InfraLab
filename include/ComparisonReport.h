#ifndef COMPARISON_REPORT_H
#define COMPARISON_REPORT_H

#include <cstddef>
#include <ostream>
#include "SimulationReport.h"

class ComparisonReport {
public:
    enum class Outcome { FirstBetter, SecondBetter, EqualCounts, TradeOff };

    // Reports must come from the same traffic and time limit.
    ComparisonReport(const SimulationReport& first, const SimulationReport& second, std::size_t requestCount, double timeLimit);
    Outcome compare() const;
    friend std::ostream& operator<<(std::ostream& os, const ComparisonReport& report);

private:
    SimulationReport firstReport;
    SimulationReport secondReport;
    std::size_t totalRequests;
    double maximumTime;
};

#endif
