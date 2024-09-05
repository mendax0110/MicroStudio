#include "../include/ShellManager.h"

#include <iostream>
#include <array>
#include <unistd.h>
#include <imgui.h>
#include <util.h>
#include <fcntl.h>

using namespace MicroStudio;

ShellManager::ShellManager() : master_fd(-1)
{
}

ShellManager::~ShellManager()
{
    if (shellThread.joinable())
    {
        shellThread.join();
    }
    if (master_fd != -1)
    {
        close(master_fd);
    }
}

void ShellManager::StartShellProcess()
{
    int slave_fd;
    pid_t pid = forkpty(&master_fd, nullptr, nullptr, nullptr);

    if (pid == 0)
    {
        execl("/bin/bash", "bash", (char *)NULL);
        _exit(1);
    }
    else if (pid > 0)
    {
        shellThread = std::thread(&ShellManager::CaptureShellOutput, this);
    }
    else
    {
        std::cerr << "Failed to fork the shell process." << std::endl;
    }
}

void ShellManager::CaptureShellOutput()
{
    std::array<char, 128> buffer;
    while (true)
    {
        ssize_t bytesRead = read(master_fd, buffer.data(), buffer.size());
        if (bytesRead > 0)
        {
            std::lock_guard<std::mutex> lock(shellOutputWindow);
            shellOutput.append(buffer.data(), bytesRead);
        }
    }
}

void ShellManager::SendCommandToShell(const std::string &command)
{
    if (master_fd != -1)
    {
        write(master_fd, command.c_str(), command.size());
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