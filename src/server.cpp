#include "Server.h"
#include <stdexcept>
#include <cmath>
#include <utility>
#include <ostream>
#include <vector>
#include <limits>


Server::Server(int id, float processingSpeed, int waitingQueueLimit, int capacity) : 
    id(id), processingSpeed(processingSpeed),waitingQueueLimit(waitingQueueLimit),capacity(capacity)
{
    if (id <= 0)
    {
        throw std::invalid_argument("ID must be positive.");
    }
    if (capacity <= 0)
    {
        throw std::invalid_argument("Capacity must be positive.");
    }
    if (processingSpeed <= 0.0f || std::isnan(processingSpeed) || std::isinf(processingSpeed))
    {
        throw std::invalid_argument("Processing speed must be positive.");
    }
    if (waitingQueueLimit < 0)
    {
        throw std::invalid_argument("Waiting queue limit cannot be negative.");
    }
}

Server::Server(const Server& other) : 
    id(other.id), processingSpeed(other.processingSpeed), waitingQueueLimit(other.waitingQueueLimit), capacity(other.capacity),
    waitingRequests(other.waitingRequests), activeRequests(other.activeRequests), completedRequests(other.completedRequests),
    rejectedRequests(other.rejectedRequests)
{
}
    
bool Server::submitRequest(Request waitingRequest,float currentTime)
{
    if(currentTime < 0.0f || std::isnan(currentTime) || std::isinf(currentTime))
    {
        throw std::invalid_argument("Current time must be a valid non-negative float.");
    }
    if(waitingRequest.getArrivalTime() > currentTime)
    {
        throw std::invalid_argument("Request arrival time cannot be in the future.");
    }
    if(waitingRequest.getState() != Request::State::PENDING)
    {
        throw std::runtime_error("Only pending requests can be added to the queue.");
    }

    waitingQueueToActive(currentTime);

    if(static_cast<int>(activeRequests.size())<getCapacity())
    {
        addRequestActive(waitingRequest, currentTime); 
        return true;
    }
    else if (static_cast<int>(waitingRequests.size()) < waitingQueueLimit)
    {
        waitingRequests.push(waitingRequest);
        return true;
    }
    else
    {
        waitingRequest.reject();
        rejectedRequests.push(waitingRequest);
        return false;
    }
}

void Server::removeWaitingRequestQueue()
{
    if (!waitingRequests.empty())
    {
        waitingRequests.pop();
    }
    else
    {
        throw std::runtime_error("No requests to remove.");
    }
}

void Server::addRequestActive(Request activeRequest, float currentTime)
{
    if (static_cast<int>(activeRequests.size()) < capacity && activeRequest.getState() == Request::State::PENDING)
    {
        activeRequest.start(currentTime);
        activeRequests.push(activeRequest);
    }
    else
    {
        throw std::runtime_error("Server is at full capacity.");
    }
}

void Server::removeRequestActive()
{
    if (!activeRequests.empty()&& activeRequests.front().getState() == Request::State::COMPLETED)
    {
        activeRequests.pop();
    }
    else
    {
        throw std::runtime_error("No requests to remove.");
    }
}

void Server::waitingQueueToActive(float currentTime)
{
    while(!waitingRequests.empty() && static_cast<int>(activeRequests.size()) < capacity)
    {
        Request nextRequest = waitingRequests.front();
        addRequestActive(nextRequest, currentTime);
        removeWaitingRequestQueue();
    }
}

void Server::completeActiveRequest(float currentTime)
{
    if(currentTime < 0.0f || std::isnan(currentTime) || std::isinf(currentTime))
    {
        throw std::invalid_argument("Current time must be a valid non-negative float.");
    }
    if (!activeRequests.empty())
    {
        std::queue<Request> tempQueue; 
        while(!activeRequests.empty())
        {
            Request& activeRequest = activeRequests.front();
            float scheduledFinishTime = activeRequest.getStartTime() + (activeRequest.getWorkUnit() / processingSpeed);
            if(currentTime >= scheduledFinishTime)
            {
                activeRequest.complete(scheduledFinishTime);
                completedRequests.push(activeRequest);
                removeRequestActive();
            }
            else
            {
                tempQueue.push(activeRequest);
                activeRequests.pop();
            }
        }
        activeRequests = std::move(tempQueue);
    }
}

float Server::nextCompletionTime() const
{
    if(activeRequests.empty())
    {
        return std::numeric_limits<float>::infinity();
    }
    float nextCompletion = std::numeric_limits<float>::infinity();
    std::queue<Request> tempQueue = activeRequests; 
    while(!tempQueue.empty())
    {
        const Request& activeRequest = tempQueue.front();
        float scheduledFinishTime = activeRequest.getStartTime() + (activeRequest.getWorkUnit() / processingSpeed);
        if(scheduledFinishTime < nextCompletion)
        {
            nextCompletion = scheduledFinishTime;
        }
        tempQueue.pop();
    }
    return nextCompletion;
}

void Server::showRequests(std::ostream& os, const std::queue<Request>& queue)
{
    std::queue<Request> tempQueue = queue; 
    while (!tempQueue.empty())
    {
        os << "    " << tempQueue.front() << "\n";
        tempQueue.pop();
    }
    if (queue.empty())
    {
        os << "    (none)\n";
    }
}

void Server::collectCompletedRequests(std::vector<Request>& completedQueue)
{
    while (!completedRequests.empty())
    {
        completedQueue.push_back(completedRequests.front());
        completedRequests.pop();
    }
}

void Server::collectRejectedRequests(std::vector<Request>& rejectedQueue)
{
    while (!rejectedRequests.empty())
    {
        rejectedQueue.push_back(rejectedRequests.front());
        rejectedRequests.pop();
    }
}

Server& Server::operator=(const Server& other)
{
    if (this != &other)
    {
        id = other.id;
        processingSpeed = other.processingSpeed;
        waitingQueueLimit = other.waitingQueueLimit;
        capacity = other.capacity;
        waitingRequests = other.waitingRequests;
        activeRequests = other.activeRequests;
        completedRequests = other.completedRequests;
        rejectedRequests = other.rejectedRequests;
    }
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Server& server)
{
    os << "Server ID: " << server.getId() << " | ";
    os << "Processing Speed: " << server.getProcessingSpeed() << " workunits/sec per request\n";
    os << "Waiting ("<<server.getWaitingQueueSize()<<"/"<<server.getWaitingQueueLimit()<<")\n";
    server.showRequests(os, server.waitingRequests);
    os << "Active ("<<server.getActiveQueueSize()<<"/"<<server.getCapacity()<<")\n";
    server.showRequests(os, server.activeRequests);
    os << "Completed ("<<server.getCompletedQueueSize()<<")\n";
    server.showRequests(os, server.completedRequests);
    return os;
}


