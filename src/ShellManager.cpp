#include "../include/ShellManager.h"

#include <iostream>
#include <array>
#include <unistd.h>
#include <imgui.h>
#include <util.h>
#include <fcntl.h>
#include <csignal>
#include <sys/wait.h>

using namespace MicroStudio;

ShellManager::ShellManager() : master_fd(-1), shellPid(-1)
{
}

ShellManager::~ShellManager()
{
    StopShellProcess();
}

void ShellManager::StartShellProcess()
{
    pid_t pid = forkpty(&master_fd, nullptr, nullptr, nullptr);

    if (pid == 0)
    {
        execl("/bin/bash", "bash", (char *)nullptr);
        _exit(1);
    }
    else if (pid > 0)
    {
        shellPid = pid;
        shellThread = std::thread(&ShellManager::CaptureShellOutput, this);
    }
    else
    {
        std::cerr << "Failed to fork the shell process." << std::endl;
    }
}

void ShellManager::StopShellProcess()
{
    if (shellPid > 0)
    {
        if (kill(shellPid, SIGTERM) == -1)
        {
            std::cerr << "Failed to send SIGTERM to shell process: " << strerror(errno) << std::endl;
        }

        int status;
        pid_t result = waitpid(shellPid, &status, WNOHANG);
        if (result == -1)
        {
            std::cerr << "Error waiting for shell process to terminate: " << strerror(errno) << std::endl;
        }
        else if (result == 0)
        {
            std::cerr << "Shell process did not terminate gracefully. Forcing termination." << std::endl;
            if (kill(shellPid, SIGKILL) == -1)
            {
                std::cerr << "Failed to send SIGKILL to shell process: " << strerror(errno) << std::endl;
            }

            result = waitpid(shellPid, &status, 0);
            if (result == -1)
            {
                std::cerr << "Error waiting for shell process to terminate: " << strerror(errno) << std::endl;
            }
        }

        if (shellThread.joinable())
        {
            shellThread.join();
        }

        if (master_fd != -1)
        {
            close(master_fd);
            master_fd = -1;
        }

        shellPid = -1;
    }
}

void ShellManager::CaptureShellOutput()
{
    std::array<char, 128> buffer{};

    while (true)
    {
        ssize_t bytesRead = read(master_fd, buffer.data(), buffer.size());

        if (bytesRead > 0)
        {
            std::lock_guard<std::mutex> lock(shellOutputWindow);
            shellOutput.append(buffer.data(), bytesRead);
        }
        else if (bytesRead == 0 || (bytesRead == -1 && errno == EIO))
        {
            break;
        }
        else if (bytesRead == -1)
        {
            std::cerr << "Error reading from shell: " << strerror(errno) << std::endl;
            break;
        }
    }
}

void ShellManager::SendCommandToShell(const std::string &command) const
{
    if (master_fd != -1)
    {
        ssize_t result = write(master_fd, command.c_str(), command.size());
        if (result == -1)
        {
            std::cerr << "Failed to write command to shell: " << strerror(errno) << std::endl;
        }
    }
}

void ShellManager::RenderShellWindow()
{
    ImGui::Begin("Terminal");

    {
        std::lock_guard<std::mutex> lock(shellOutputWindow);
        ImGui::TextUnformatted(shellOutput.c_str());
    }

    static char inputBuffer[256] = "";

    if (ImGui::InputText("Command", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        std::string command = std::string(inputBuffer) + "\n";
        SendCommandToShell(command);
        inputBuffer[0] = '\0';
    }

    ImGui::End();
}
