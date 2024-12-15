#pragma once

#include <string>
#include <thread>
#include <mutex>
#if defined(__APPLE__) || (__LINUX)
#include <util.h>
#endif
namespace MicroStudio
{
    class ShellManager
    {
    public:
        ShellManager();
        ~ShellManager();

        void StartShellProcess();
        void SendCommandToShell(const std::string &command) const;
        void RenderShellWindow();
        void StopShellProcess();

    private:
        int master_fd;
        std::thread shellThread;
        std::mutex shellOutputWindow;
        std::string shellOutput;
#if defined(__APPLE__) || defined(__LINUX__)
        pid_t shellPid{};
#elif defined(_WIN32)
        int shellPid{};
#endif

        void CaptureShellOutput();
    };
}

