// Main ships no test framework on purpose: plain assert / static_assert only.
// Full framework examples live on tests/gtest and tests/Boost-ext-ut branches.

// assert() disappears when NDEBUG is defined (Release, RelWithDebInfo): the
// binary would exit 0 having checked nothing, and CI would call that a pass.
// Keep the checks on in every build type.
#undef NDEBUG
#include <cassert>
#include <stdexcept>
#include <sstream>

#include "Request.h"

int main()
{
    bool caught = false;
    try
    {
        Request r1(1, 0.0f, 1);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected valid Request to not throw");
    caught = false;
    try
    {
        Request r2(1, 5.0f, 10);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected valid Request to not throw");
    caught = false;
    try
    {
        Request r3(0, 0.0f, 1);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for invalid Request id");
    caught = false;
    try
    {
        Request r4(-1, 0.0f, 1);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for invalid Request id");
    caught = false;
    try
    {
        Request r5(1, -1.0f, 1);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for invalid Request arrivalTime");
    caught = false;
    try
    {
        Request r6(1, 0.0f, 0);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for invalid Request workUnit");
    caught = false;
    try
    {
        Request r7(1, 0.0f, -1);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for invalid Request workUnit");
    caught = false;
    // test start() method ///////////////////////////////////////////

    try
    {
        Request r8(1, 5.0f, 10);
        r8.start(4.0f); // start time before arrival time
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for starting Request before arrival time");
    caught = false;
    try
    {
        Request r9(1, 5.0f, 10);
        r9.start(5.0f); // valid start time
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected valid start time to not throw");
    caught = false;
    Request r10(1, 5.0f, 10);
    try
    {
        r10.start(3.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for starting Request before arrival time");
    caught = false;
    try
    {
        r10.start(5.0f); // valid start time
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected valid start time to not throw after previous failed start");
    caught = false;
    try
    {
        r10.start(6.0f); // starting again after already started
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for starting Request again after it has already started");

    // test complete() method ///////////////////////////////////////////
    caught = false;
    try
    {
        Request r11(1, 5.0f, 10);
        r11.complete(4.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for completing Request before progress");
    caught = false;
    try
    {
        Request r12(1, 5.0f, 10);
        r12.start(5.0f);
        r12.complete(4.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for completing Request before start time");
    caught = false;
    Request r13(1, 5.0f, 10);
    try
    {
        r13.start(5.25f);
        r13.complete(5.75f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected valid completion time to not throw");
    try
    {
        r13.complete(7.0f); // completing again after already completed
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for completing Request again after it has already been completed");
    caught = false;
    Request r14(1, 5.0f, 10);
    try
    {
        r14.start(5.0f);
        r14.complete(5.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for completing Request at the same time as start time");
    caught = false;
    try
    {
        r14.complete(7.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected valid completion time to not throw after previous failed complete");

    // test waitingTime() method ///////////////////////////////////////////
    caught = false;
    Request r15(1, 5.0f, 10);
    try
    {
        r15.waitingTime();
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for waiting time of a pending Request");
    caught = false;
    r15.start(5.25f);
    try
    {
        r15.waitingTime();
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected valid waiting time of a started Request to not throw");
    caught = false;
    r15.complete(7.0f);
    try
    {
        assert(r15.waitingTime() == 0.25f && "Expected waiting time to equal start time minus arrival time");
        assert(r15.waitingTime() == 0.25f && "Expected repeated waiting time calls to preserve the result");
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected waiting time of a completed Request to not throw");
    caught = false;
    Request r16(1, 5.0f, 10);
    r16.start(5.0f);
    r16.complete(7.0f);
    try
    {
        assert(r16.waitingTime() == 0.0f && "Expected zero waiting time when started on arrival");
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected zero waiting time to not throw");
    caught=false;
    // test responseTime() method ///////////////////////////////////////////
    try{
        r16.responseTime();
    }
    catch(const std::invalid_argument&){
        caught=true;
    }
    assert(!caught && "Expected valid response time of a completed Request to not throw");
    assert(r16.responseTime() == 2.0f && "Expected response time to equal finish time minus arrival time");
    assert(r16.responseTime() == 2.0f && "Expected repeated response time calls to preserve the result");
    caught=false;
    Request r17(1, 5.0f, 10);
    try{
        r17.responseTime();
    }
    catch(const std::invalid_argument&){
        caught=true;
    }
    assert(caught && "Expected exception for response time of a pending Request");
    r17.start(5.0f);
    caught=false;
    try{
        r17.responseTime();
    }
    catch(const std::invalid_argument&){
        caught=true;
    }
    assert(caught && "Expected exception for response time of a started Request");
    r17.complete(7.0f);
    caught=false;
    try{
        assert(r17.responseTime() == 2.0f && "Expected response time to equal finish time minus arrival time");
    }
    catch(const std::invalid_argument&){
        caught=true;
    }
    assert(!caught && "Expected valid response time of a completed Request to not throw");
    assert(r17.responseTime() == 2.0f && "Expected repeated response time calls to preserve the result");
    caught= false;
    Request r18(1, 5.0f, 10);
    r18.start(6.0f);
    r18.complete(8.0f);
    try{
        assert(r18.responseTime() == 3.0f && "Expected response time to equal finish time minus arrival time");
        assert(r18.waitingTime() == 1.0f && "Expected waiting time to equal start time minus arrival time"); 
    }
    catch(const std::invalid_argument&){
        caught=true;
    }
    assert(!caught && "Expected valid response and waiting times of a completed Request to not throw");
    caught = false;


    // test reject() method ///////////////////////////////////////////
    Request r19(1, 5.0f, 10);
    try
    {
        r19.reject();
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected rejecting a pending Request to not throw");
    caught = false;
    try
    {
        r19.reject();
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for rejecting a Request again");
    caught = false;
    try
    {
        r19.start(6.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for starting a rejected Request");
    caught = false;
    try
    {
        r19.complete(7.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for completing a rejected Request");
    caught = false;
    try
    {
        r19.waitingTime();
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for waiting time of a rejected Request");
    caught = false;
    try
    {
        r19.responseTime();
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for response time of a rejected Request");
    Request r20(1, 5.0f, 10);
    r20.start(5.25f);
    caught = false;
    try
    {
        r20.reject();
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for rejecting a Request in progress");
    assert(r20.waitingTime() == 0.25f && "Expected failed rejection to preserve waiting time");
    caught = false;
    try
    {
        r20.complete(7.0f);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(!caught && "Expected completion after failed rejection to succeed");
    caught = false;
    try
    {
        r20.reject();
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught && "Expected exception for rejecting a completed Request");
    assert(r20.waitingTime() == 0.25f && "Expected failed rejection to preserve completed waiting time");
    assert(r20.responseTime() == 2.0f && "Expected failed rejection to preserve response time");

    // test operator<< ///////////////////////////////////////////
    Request r21(21, 5.0f, 10);
    std::ostringstream pendingOutput;
    pendingOutput << r21;
    assert(pendingOutput.str() ==
        "Request ID : 21| Arrival Time 5| Work Unit: 10| State: PENDING" &&
        "Expected pending Request output without timing results");

    r21.start(5.25f);
    std::ostringstream startedOutput;
    startedOutput << r21;
    assert(startedOutput.str() ==
        "Request ID : 21| Arrival Time 5| Work Unit: 10| State: IN_PROGRESS"
        "| Start Time: 5.25| Waiting Time: 0.25" &&
        "Expected started Request output with start and waiting times");

    r21.complete(7.0f);
    std::ostringstream completedOutput;
    completedOutput << r21;
    assert(completedOutput.str() ==
        "Request ID : 21| Arrival Time 5| Work Unit: 10| State: COMPLETED"
        "| Start Time: 5.25| Finish Time: 7| Waiting Time: 0.25| Response Time: 2" &&
        "Expected completed Request output with all timing results");
    assert(r21.waitingTime() == 0.25f && r21.responseTime() == 2.0f &&
        "Expected output to preserve Request timing results");

    Request r22(22, 1.5f, 4);
    r22.reject();
    std::ostringstream rejectedOutput;
    rejectedOutput << r22;
    assert(rejectedOutput.str() ==
        "Request ID : 22| Arrival Time 1.5| Work Unit: 4| State: REJECTED" &&
        "Expected rejected Request output without timing results");

    const Request r23(23, 0.0f, 1);
    std::ostringstream chainedOutput;
    std::ostream& returnedStream = (chainedOutput << r23);
    assert(&returnedStream == &chainedOutput && "Expected operator<< to return the supplied stream");
    returnedStream << " | " << r22;
    assert(chainedOutput.str() ==
        "Request ID : 23| Arrival Time 0| Work Unit: 1| State: PENDING | "
        "Request ID : 22| Arrival Time 1.5| Work Unit: 4| State: REJECTED" &&
        "Expected chained output to support const Requests and multiple insertions");

    return 0;
}
