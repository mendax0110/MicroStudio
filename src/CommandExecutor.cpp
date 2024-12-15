#include "../include/CommandExecutor.h"
#include <sstream>
#include <iostream>
#include <mutex>

using namespace MicroStudio;

void CommandExecutor::Execute(const std::string& command, std::string& output, std::atomic<bool>& doneFlag, std::mutex& outputMutex)
{
    std::ostringstream oss;
#if defined(__APPLE__) || defined(__LINUX__)
    FILE* pipe = popen(command.c_str(), "r");
#elif defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#endif
    if (!pipe)
    {
        std::cerr << "Failed to execute command: " << command << std::endl;
        doneFlag.store(true);
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        oss << buffer;
    }

#if defined(__APPLE__) || defined(__LINUX__)
    pclose(pipe);
#elif defined(_WIN32)
    _pclose(pipe);
#endif

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        output = oss.str();
    }
    doneFlag.store(true);
}
