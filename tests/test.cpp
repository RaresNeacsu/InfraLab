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


int main()
{
    testFileLoading();
    testRequest();
    testServer();
    testSimulationReport();
    testSimulation();
    testComparisonReport();
    return 0;
}
