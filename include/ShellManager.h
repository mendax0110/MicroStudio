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
        void SendCommandToShell(const std::string &command);
        void RenderShellWindow();

    private:
        int master_fd;
        std::thread shellThread;
        std::mutex shellOutputWindow;
        std::string shellOutput;

        void CaptureShellOutput();
    };
}

