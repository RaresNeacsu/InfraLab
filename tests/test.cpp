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


int main()
{
    testFileLoading();
    testRequest();
    testServer();
    testSimulationReport();
    testSimulation();
    return 0;
}
