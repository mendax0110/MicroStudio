#include "../include/DebuggerHelper.h"
#include <iostream>
#include <cstdlib>

#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#endif

#if defined(_WIN32)
// include winnt header for windows
#include <Windows.h>
#include <process.h>
#include <processthreadsapi.h>
#endif

#include <io.h>
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
#if defined(__APPLE__) || defined(__linux__)
    if (access(debuggerExecutable.c_str(), X_OK) != 0)
    {
#elif defined(_WIN32)
    if (_access(debuggerExecutable.c_str(), 0) != 0)
    {
#endif
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

#if defined(__APPLE__) || defined(__linux__)
    pid_t pid = fork();
#endif

#if defined(_WIN32)
    int pid = _spawnl(_P_NOWAIT, debuggerExecutable.c_str(), "lldb", executablePath.c_str(), nullptr);
#endif
    if (pid == 0)
    {
#if defined(__APPLE__) || defined(__linux__)
        execlp("/usr/bin/lldb", "lldb", executablePath.c_str(), nullptr);
#elif defined (_WIN32)
        execlp("C:\\Program Files\\LLVM\\bin\\lldb.exe", "lldb", executablePath.c_str(), nullptr);
#endif
        std::cerr << "Failed to start debugger." << std::endl;
        _exit(1);
    }
    else if (pid > 0)
    {
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
#if defined(__APPLE__) || defined(__linux__)
        if (kill(debugProcessPid, SIGTERM) != 0)
        {
#elif defined(_WIN32)
        if (TerminateProcess((HANDLE)debugProcessPid, 0) == 0)
        {
#endif
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