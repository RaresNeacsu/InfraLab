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
    Request(int id, double arrivalTime, double workUnit);
    void start(double requestStartTime);
    void complete(double requestFinishTime);
    void reject();
    double waitingTime() const;
    double responseTime() const;
    int getId() const { return id; }
    double getWorkUnit() const { return workUnit; }
    Request::State getState() const { return state; }
    double getStartTime() const { return startTime; }
    double getArrivalTime() const { return arrivalTime; }
    ~Request() = default;
    friend std::ostream& operator<<(std::ostream& os, const Request& request);
private:

    int id;
    double arrivalTime;
    double workUnit;
    State state;
    double startTime=-1.0f;
    double finishTime=-1.0f;
};

std::ostream& operator<<(std::ostream& os, const Request& request);

#endif // REQUEST_H
