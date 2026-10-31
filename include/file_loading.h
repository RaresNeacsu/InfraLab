#ifndef FILE_LOADING_H
#define FILE_LOADING_H

#include <fstream>
#include <string>
#include <stdexcept>
#include "Server.h"
#include "Request.h"
#include <vector>


class fileLoading{
    public:
        static std::ifstream loadFromFile(const std::string& filename);
        static std::ofstream saveToFile(const std::string& filename);
        static std::vector<Request> loadRequestsFromFile(std::istream &file);
        static std::vector<Server> loadServersFromFile(std::istream &file);
};



#endif // FILE_LOADING_H
