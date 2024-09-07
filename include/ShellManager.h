#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <util.h>

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
        pid_t shellPid{};

        void CaptureShellOutput();
    };
}

