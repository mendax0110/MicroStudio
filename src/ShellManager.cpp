#include "../include/ShellManager.h"

#include <iostream>
#include <array>
#include <imgui.h>

#if defined(__APPLE__) || defined(__LINUX__)
#include <unistd.h>
#include <util.h>
#include <fcntl.h>
#endif

#if defined(_WIN32)
#include <wtypes.h>
#include <processthreadsapi.h>
#include <io.h>
#endif


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
    int slave_fd;
#if defined(__APPLE__) || defined(__LINUX__)
    pid_t pid = forkpty(&master_fd, nullptr, nullptr, nullptr);
#endif

#if defined(_WIN32)
    int pid = _getpid();
#endif

    if (pid == 0)
    {
#if defined(__APPLE__) || defined(__LINUX__)
        execl("/bin/bash", "bash", (char *)NULL);
        _exit(1);
#endif

#if defined(_WIN32)
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		CreateProcess("C:\\Windows\\System32\\cmd.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
#endif
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
#if defined(__APPLE__) || defined(__LINUX__)
        if (kill(shellPid, SIGTERM) == -1)
#elif defined(_WIN32)
        if (TerminateProcess((HANDLE)shellPid, 0) == 0)
        {
#endif
            std::cerr << "Failed to send SIGTERM to shell process: " << strerror(errno) << std::endl;
        }

        int status;
#if defined(__APPLE__) || defined(__LINUX__)
        pid_t result = waitpid(shellPid, &status, WNOHANG);
#elif defined(_WIN32)
        DWORD result = WaitForSingleObject((HANDLE)shellPid, 0);
#endif
        if (result == -1)
        {
            std::cerr << "Error waiting for shell process to terminate: " << strerror(errno) << std::endl;
        }
        else if (result == 0)
        {
            std::cerr << "Shell process did not terminate gracefully. Forcing termination." << std::endl;
#if defined(__APPLE__) || defined(__LINUX__)
            if (kill(shellPid, SIGKILL) == -1)
#elif defined(_WIN32)
            if (TerminateProcess((HANDLE)shellPid, 0) == 0)
#endif
            {
                std::cerr << "Failed to send SIGKILL to shell process: " << strerror(errno) << std::endl;
            }

#if defined(__APPLE__) || defined(__LINUX__)
            result = waitpid(shellPid, &status, 0);
#elif defined(_WIN32)
            result = WaitForSingleObject((HANDLE)shellPid, INFINITE);
#endif
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
#if defined(__APPLE__) || defined(__LINUX__)
            close(master_fd);
#elif defined(_WIN32)
            _close(master_fd);
#endif
            master_fd = -1;
        }

        shellPid = -1;
    }
}

void ShellManager::CaptureShellOutput()
{
    std::array<char, 128> buffer;
    while (true)
    {
#if defined(__APPLE__) || defined(__LINUX__)
        ssize_t bytesRead = read(master_fd, buffer.data(), buffer.size());
#endif

#if defined(_WIN32)
        DWORD bytesRead;
        auto master_fd_h = reinterpret_cast<HANDLE>(master_fd);
        ReadFile(master_fd_h, buffer.data(), buffer.size(), &bytesRead, NULL);
#endif
        if (bytesRead > 0)
        {
            std::lock_guard<std::mutex> lock(shellOutputWindow);
            shellOutput.append(buffer.data(), bytesRead);
        }
    }
}


void ShellManager::SendCommandToShell(const std::string &command) const
{
    if (master_fd != -1)
    {
#if defined(__APPLE__) || defined(__LINUX__)
        write(master_fd, command.c_str(), command.size());
#endif

#if defined(_WIN32)
        DWORD bytesWritten;
		auto master_fd_h = reinterpret_cast<HANDLE>(master_fd);
		WriteFile(master_fd_h, command.c_str(), command.size(), &bytesWritten, NULL);
#endif
	
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
