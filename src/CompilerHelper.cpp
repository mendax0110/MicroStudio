#include "../include/CompilerHelper.h"
#include <iostream>
#include <fstream>
#include <array>
#include <memory>

using namespace MicroStudio;

CompilerHelper::CompilerHelper() : compileDone(false) {}

CompilerHelper::~CompilerHelper()
{
    if (compileThread.joinable())
    {
        compileThread.join();
    }
}

void CompilerHelper::Compile(const std::string& filePath, const std::string& fileName)
{
    compileDone.store(false);
    buildOutput.clear();

    GenerateCMakeLists(filePath, fileName);

    std::string cleanBuildCommand = "rm -rf build";
    std::string buildCommand = "cmake -S . -B build && cmake --build build --config Release";

    compileThread = std::thread(&CompilerHelper::ExecuteBuild, this, cleanBuildCommand, buildCommand);
}

void CompilerHelper::RunCompiledCode(const std::string& executableName)
{
    std::string command = "./build/" + executableName;
    std::thread runThread([command]() {
        system(command.c_str());
    });
    runThread.detach();
}

std::string CompilerHelper::GetBuildOutput()
{
    std::lock_guard<std::mutex> lock(outputMutex);
    return buildOutput;
}

bool CompilerHelper::IsCompileDone() const
{
    return compileDone.load();
}

void CompilerHelper::GenerateCMakeLists(const std::string& filePath, const std::string& fileName)
{
    std::string cmakeListsContent = R"(
        cmake_minimum_required(VERSION 3.10)
        project(DynamicBuild)

        set(CMAKE_CXX_STANDARD 17)

        add_executable(DynamicBuild )" + filePath + "/" + fileName + R"()
    )";

    std::ofstream cmakeListsFile("CMakeLists.txt");
    if (cmakeListsFile.is_open())
    {
        cmakeListsFile << cmakeListsContent;
        cmakeListsFile.close();
        std::cout << "[INFO] Generated CMakeLists.txt with content:\n" << cmakeListsContent << std::endl;
    }
    else
    {
        std::cerr << "[ERROR] Failed to open CMakeLists.txt for writing." << std::endl;
    }
}

void CompilerHelper::ExecuteBuild(const std::string& cleanCommand, const std::string& buildCommand)
{
    std::array<char, 128> buffer{};
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> cleanPipe(popen(cleanCommand.c_str(), "r"), pclose);
    if (cleanPipe)
    {
        while (fgets(buffer.data(), buffer.size(), cleanPipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        std::cout << "[INFO] Clean command output:\n" << result << std::endl;
    }
    else
    {
        std::cerr << "[ERROR] Clean command failed: " << cleanCommand << std::endl;
    }

    result.clear();

    std::unique_ptr<FILE, decltype(&pclose)> buildPipe(popen(buildCommand.c_str(), "r"), pclose);
    if (!buildPipe)
    {
        result = "popen() failed!";
        std::cerr << "[ERROR] Build command failed to start: " << buildCommand << std::endl;
    }
    else
    {
        while (fgets(buffer.data(), buffer.size(), buildPipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        std::cout << "[INFO] Build command output:\n" << result << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        buildOutput = result;
    }
    compileDone.store(true);
}
