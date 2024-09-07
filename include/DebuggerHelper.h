#pragma once

#include <string>

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
        pid_t debugProcessPid;
        std::string debuggerExecutable;

        void DetermineDebugger();
    };
}
