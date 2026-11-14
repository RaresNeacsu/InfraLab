#undef NDEBUG
#include <cassert>
#include <fstream>
#include <sstream>
#include <stdexcept>


void testRequest();
void testServer();
void testSimulationReport();
void testSimulation();
void testFileLoading();
void testComparisonReport();
void testRoutingStrategy();


int main()
{
    testFileLoading();
    testRequest();
    testServer();
    testSimulationReport();
    testSimulation();
    testComparisonReport();
    testRoutingStrategy();
    return 0;
}
