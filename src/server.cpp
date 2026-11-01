#include "Server.h"
#include <stdexcept>
#include <cmath>
#include <utility>
#include <ostream>
#include <vector>
#include <limits>
#include <algorithm>

namespace {
double scheduledFinishTime(const Request& request, double processingSpeed)
{
    const double duration = request.getWorkUnit() / processingSpeed;
    const double finish = request.getStartTime() + duration;
    if (!std::isfinite(duration) || duration <= 0.0 ||
        !std::isfinite(finish) || finish <= request.getStartTime()) {
        throw std::overflow_error("Request processing time cannot be represented accurately.");
    }
    return finish;
}
}


Server::Server(int id, double processingSpeed, int waitingQueueLimit, int capacity) :
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
    completedRequestsCount(other.completedRequestsCount), rejectedRequestsCount(other.rejectedRequestsCount),
    totalRequestsCount(other.totalRequestsCount), maxWaitingQueueSize(other.maxWaitingQueueSize),
    waitingRequests(other.waitingRequests), activeRequests(other.activeRequests), completedRequests(other.completedRequests),
    rejectedRequests(other.rejectedRequests)
{
}

bool Server::canAcceptRequest() const
{
    return activeRequests.size() + waitingRequests.size()
        < static_cast<std::size_t>(capacity) + static_cast<std::size_t>(waitingQueueLimit);
}

bool Server::submitRequest(Request waitingRequest,double currentTime)
{
    if(currentTime < 0.0f || std::isnan(currentTime) || std::isinf(currentTime))
    {
        throw std::invalid_argument("Current time must be a valid non-negative double.");
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
        ++totalRequestsCount;
        return true;
    }
    else if (static_cast<int>(waitingRequests.size()) < waitingQueueLimit)
    {
        waitingRequests.push(waitingRequest);
        ++totalRequestsCount;
        this->maxWaitingQueueSize = std::max(this->maxWaitingQueueSize, static_cast<int>(waitingRequests.size()));
        return true;
    }
    else
    {
        waitingRequest.reject();
        this->rejectedRequestsCount++;
        rejectedRequests.push(waitingRequest);
        ++totalRequestsCount;
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

void Server::addRequestActive(Request activeRequest, double currentTime)
{
    if (static_cast<int>(activeRequests.size()) < capacity && activeRequest.getState() == Request::State::PENDING)
    {
        activeRequest.start(currentTime);
        scheduledFinishTime(activeRequest, processingSpeed);
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

void Server::waitingQueueToActive(double currentTime)
{
    while(!waitingRequests.empty() && static_cast<int>(activeRequests.size()) < capacity)
    {
        Request nextRequest = waitingRequests.front();
        addRequestActive(nextRequest, currentTime);
        removeWaitingRequestQueue();
    }
}

void Server::completeActiveRequest(double currentTime)
{
    if(currentTime < 0.0f || std::isnan(currentTime) || std::isinf(currentTime))
    {
        throw std::invalid_argument("Current time must be a valid non-negative double.");
    }
    if (!activeRequests.empty())
    {
        std::queue<Request> tempQueue;
        while(!activeRequests.empty())
        {
            Request& activeRequest = activeRequests.front();
            const double finishTime = scheduledFinishTime(activeRequest, processingSpeed);
            if(currentTime >= finishTime)
            {
                activeRequest.complete(finishTime);
                this->completedRequestsCount++;
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

double Server::nextCompletionTime() const
{
    if(activeRequests.empty())
    {
        return std::numeric_limits<double>::infinity();
    }
    double nextCompletion = std::numeric_limits<double>::infinity();
    std::queue<Request> tempQueue = activeRequests;
    while(!tempQueue.empty())
    {
        const Request& activeRequest = tempQueue.front();
        const double finishTime = scheduledFinishTime(activeRequest, processingSpeed);
        if(finishTime < nextCompletion)
        {
            nextCompletion = finishTime;
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

void Server::collectCompletedRequests(std::vector<Request>& completedQueue) const
{
    auto history = completedRequests;
    std::vector<Request> snapshot;
    snapshot.reserve(history.size());
    while (!history.empty()) {
        snapshot.push_back(history.front());
        history.pop();
    }
    completedQueue = std::move(snapshot);
}

void Server::collectRejectedRequests(std::vector<Request>& rejectedQueue) const
{
    auto history = rejectedRequests;
    std::vector<Request> snapshot;
    snapshot.reserve(history.size());
    while (!history.empty()) {
        snapshot.push_back(history.front());
        history.pop();
    }
    rejectedQueue = std::move(snapshot);
}

void Server::collectServerStatistics(int& completedCount, int& rejectedCount, int& totalCount, int& queuePeak) const
{
    completedCount = this->completedRequestsCount;
    rejectedCount = this->rejectedRequestsCount;
    totalCount = this->totalRequestsCount;
    queuePeak = this->maxWaitingQueueSize;
}

Server& Server::operator=(const Server& other)
{
    if (this != &other)
    {
        id = other.id;
        processingSpeed = other.processingSpeed;
        waitingQueueLimit = other.waitingQueueLimit;
        capacity = other.capacity;
        completedRequestsCount = other.completedRequestsCount;
        rejectedRequestsCount = other.rejectedRequestsCount;
        totalRequestsCount = other.totalRequestsCount;
        maxWaitingQueueSize = other.maxWaitingQueueSize;
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
    os << "Completed ("<<server.completedRequestsCount<<")\n";
    server.showRequests(os, server.completedRequests);
    os << "Rejected ("<<server.rejectedRequestsCount<<")\n";
    os << "Total ("<<server.totalRequestsCount<<")\n";
    os << "Max Waiting Queue Size ("<<server.maxWaitingQueueSize<<")\n";

    return os;
}


