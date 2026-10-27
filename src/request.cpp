#include "Request.h"
#include <cmath>
#include <stdexcept>

Request::Request(int id, float arrivalTime, float workUnit) : id(id), arrivalTime(arrivalTime), workUnit(workUnit), state(State::PENDING)
{
    if (id <= 0)
    {
        throw std::invalid_argument("ID must be positive.");
    }
    if (arrivalTime < 0.0f || std::isnan(arrivalTime) || std::isinf(arrivalTime))
    {
        throw std::invalid_argument("Arrival time must be a valid non-negative float.");
    }
    if (workUnit <= 0.0f || std::isnan(workUnit) || std::isinf(workUnit))
    {
        throw std::invalid_argument("Work unit must be a valid positive float.");
    }
}

void Request::start(float requestStartTime)
{
    if(requestStartTime < 0.0f || std::isnan(requestStartTime)|| std::isinf(requestStartTime))
    {
        throw std::invalid_argument("Start time must be a valid non-negative float.");
    }
    if(state == State::PENDING)
    {
        if(requestStartTime < arrivalTime)
        {
            throw std::invalid_argument("Start time cannot be before arrival time.");
        }
        this->startTime = requestStartTime;
        state = State::IN_PROGRESS;
    }
    else
    {
        throw std::invalid_argument("Request can only be started if it is in PENDING state.");
    }
}
void Request::complete(float requestFinishTime)
{
    if(requestFinishTime < 0.0f || std::isnan(requestFinishTime) || std::isinf(requestFinishTime))
    {
        throw std::invalid_argument("Finish time must be a valid non-negative float.");
    }
    if(state == State::IN_PROGRESS)
    {
        if(requestFinishTime <= startTime)
        {
            throw std::invalid_argument("Finish time must be after start time.");
        }
        state = State::COMPLETED;
        this->finishTime = requestFinishTime;  
    }
    else
    {
        throw std::invalid_argument("Request can only be completed if it is in IN_PROGRESS state.");
    }
}

void Request::reject()
{
    if(state == State::PENDING)
    {
        state = State::REJECTED;
    }
    else
    {
        throw std::invalid_argument("Request can only be rejected if it is in PENDING state.");
    }
}

float Request::waitingTime() const
{
    if(state == State::IN_PROGRESS || state == State::COMPLETED)
    {
        return startTime - arrivalTime;
    }
    else
    {
        throw std::invalid_argument("Waiting time can only be calculated for started requests.");
    }   
}

float Request::responseTime() const
{
    if(state == State::COMPLETED)
    {
        return finishTime - arrivalTime;
    }
    else
    {
        throw std::invalid_argument("Response time can only be calculated for completed requests.");
    }
}

std::ostream& operator<<(std::ostream& os, const Request& request)
{
    os << "Request #" << request.id << " | Arrival: " << request.arrivalTime
       << " s | Work units: " << request.workUnit << " | State: ";
    switch (request.state)
    {
        case Request::State::PENDING:
            os << "PENDING";
            break;
        case Request::State::IN_PROGRESS:
            os << "IN_PROGRESS";
            break;
        case Request::State::COMPLETED:
            os << "COMPLETED";
            break;
        case Request::State::REJECTED:
            os << "REJECTED";
            break;
    }
    if(request.state == Request::State::IN_PROGRESS || request.state == Request::State::COMPLETED)
    {
        os << "\n    Start: " << request.startTime;
    }
    if(request.state == Request::State::COMPLETED)
    {
        os << " s | Finish: " << request.finishTime;
    }
    if(request.state == Request::State::IN_PROGRESS || request.state == Request::State::COMPLETED)
    {
        os << " s | Waiting: " << request.waitingTime();
    }
    if(request.state == Request::State::COMPLETED)
    {
        os << " s | Response: " << request.responseTime();
    }
    if (request.state == Request::State::IN_PROGRESS || request.state == Request::State::COMPLETED)
    {
        os << " s";
    }
    return os;
}
