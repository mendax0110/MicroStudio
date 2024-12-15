#pragma once

#include <string>
#include <iostream>

namespace MicroStudio
{
    class DebuggerHelper
    {
    public:
        DebuggerHelper();
        ~DebuggerHelper();

        void Initialize();
        bool StartDebugging(const std::string& executablePath);
        void StopDebugging();
        bool IsDebugging() const;
        std::string GetStatus() const;

    private:
        bool isDebugging;
#if defined(__APPLE__) || defined(__LINUX__)
        pid_t debugProcessPid;
#elif defined(_WIN32)
        int debugProcessPid;
#endif
        std::string debuggerExecutable;

        void DetermineDebugger();
    };
}
