#pragma once

#include <string>
#include <atomic>
#include <mutex>

namespace MicroStudio
{
    class CommandExecutor
    {
    public:
        static void Execute(const std::string& command, std::string& output, std::atomic<bool>& doneFlag, std::mutex& outputMutex);
    };
}
