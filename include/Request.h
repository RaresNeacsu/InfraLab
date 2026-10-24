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
    int getId() const { return id; }
    int getWorkUnit() const { return workUnit; }
    Request::State getState() const { return state; }
    float getStartTime() const { return startTime; }
    float getArrivalTime() const { return arrivalTime; }
    ~Request() = default;
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