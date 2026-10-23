#ifndef REQUEST_H
#define REQUEST_H
#include <iostream>
class Request
{
public:
    enum class State
    {
        PENDING,
        IN_PROGRESS,
        COMPLETED,
        REJECTED
    };
    Request(int id, float arrivalTime, int workUnit);
    void start(float requestStartTime);
    void complete(float requestFinishTime);
    void reject();
    float waitingTime() const;
    float responseTime() const;
    friend std::ostream& operator<<(std::ostream& os, const Request& request);
private:

    int id;          
    float arrivalTime; 
    int workUnit;    
    State state;
    float startTime=-1.0f;
    float finishTime=-1.0f;
};

std::ostream& operator<<(std::ostream& os, const Request& request);
#endif // REQUEST_H