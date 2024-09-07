#include "../include/DebuggerHelper.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <csignal>

using namespace MicroStudio;

DebuggerHelper::DebuggerHelper() : isDebugging(false), debugProcessPid(-1)
{
    DetermineDebugger();
}

DebuggerHelper::~DebuggerHelper()
{
    StopDebugging();
}

void DebuggerHelper::DetermineDebugger()
{
    debuggerExecutable = "/usr/bin/lldb";
}

void DebuggerHelper::Initialize()
{
    if (access(debuggerExecutable.c_str(), X_OK) != 0)
    {
        std::cerr << "Debugger executable not found, wrong path or no executable: " << debuggerExecutable << std::endl;
        return;
    }
    std::cout << "Debugger initialized: " << debuggerExecutable << std::endl;
}

bool DebuggerHelper::StartDebugging(const std::string &executablePath)
{
    if (isDebugging)
    {
        std::cerr << "Debugger is already running." << std::endl;
        return false;
    }

    std::string command = debuggerExecutable + " " + executablePath;
    std::cout << "Start debugging: " << command << std::endl;

    pid_t pid = fork();
    if (pid == 0)
    {
        execlp("/usr/bin/lldb", "lldb", executablePath.c_str(), nullptr);
        std::cerr << "Failed to start debugger." << std::endl;
        _exit(1);
    }
    else if (pid > 0)
    {
        debugProcessPid = pid;
        isDebugging = true;
        return true;
    }
    else
    {
        std::cerr << "Failed to fork process." << std::endl;
        return false;
    }
}

void DebuggerHelper::StopDebugging()
{
    if (!isDebugging)
    {
        std::cerr << "No debugging session to stop." << std::endl;
        return;
    }

    if (debugProcessPid != -1)
    {
        if (kill(debugProcessPid, SIGTERM) != 0)
        {
            std::cerr << "Failed to terminate the debugger process." << std::endl;
        }
        else
        {
            std::cout << "Debugger process terminated" << std::endl;
        }
        debugProcessPid = -1;
    }

    std::cout << "Stopping debugging session." << std::endl;
    isDebugging = false;
}

bool DebuggerHelper::IsDebugging() const
{
    return isDebugging;
}

std::string DebuggerHelper::GetStatus() const
{
    if (isDebugging)
    {
        return "Debugging active.";
    }
    else
    {
        return "Not debugging";
    }
}