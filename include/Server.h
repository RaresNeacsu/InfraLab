#ifndef SERVER_H
#define SERVER_H
#include <queue>
#include "Request.h"
#include <ostream>
#include <vector>
class Server{
    private:
        void removeWaitingRequestQueue();
        void removeRequestActive();
        void addRequestActive(Request waitingRequest,float currentTime);
        static void showRequests(std::ostream& os, const std::queue<Request>& queue);
    public: 
        Server(int id, float processingSpeed, int waitingQueueLimit, int capacity);
        ~Server() = default;
        int getId() const { return id; }
        int getCapacity() const { return capacity; }
        float getProcessingSpeed() const { return processingSpeed; }
        int getWaitingQueueLimit() const { return waitingQueueLimit; }
        int getWaitingQueueSize() const { return waitingRequests.size(); }
        int getActiveQueueSize() const { return activeRequests.size(); }
        int getCompletedQueueSize() const { return completedRequests.size(); }

        bool submitRequest(Request waitingRequest,float currentTime);
        void waitingQueueToActive(float currentTime);
        void completeActiveRequest(float currentTime);
        float nextCompletionTime() const;
        
        friend std::ostream& operator<<(std::ostream& os, const Server& server);

        void collectCompletedRequests(std::vector<Request>& completedQueue);
        void collectRejectedRequests(std::vector<Request>& rejectedQueue);
    private:
        int id;
        float processingSpeed; //workunits per second
        int waitingQueueLimit; //maximum number of requests that can be in wait simultaneously
        //current number of requests being queued - waitingRequests.size()
        int capacity; //maximum number of requests that can be processed simultaneously
        //current number of requests being processed - activeRequests.size()
        std::queue<Request> waitingRequests; //list of requests being queued
        std::queue<Request> activeRequests; //list of requests being processed
        std::queue<Request> completedRequests; //list of requests that have been completed
        std::queue<Request> rejectedRequests; //list of requests that have been rejected

    };

std::ostream& operator<<(std::ostream& os, const Server& server);
#endif // SERVER_H