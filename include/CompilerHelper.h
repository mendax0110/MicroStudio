#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <atomic>

namespace MicroStudio
{
    class CompilerHelper
    {
    public:
        CompilerHelper();
        ~CompilerHelper();

        void Compile(const std::string& filePath, const std::string& fileName);
        void RunCompiledCode(const std::string& executableName);
        std::string GetBuildOutput();

        bool IsCompileDone() const;

    private:
        void GenerateCMakeLists(const std::string& filePath, const std::string& fileName);
        void ExecuteBuild(const std::string& cleanCommand, const std::string& buildCommand);

        std::atomic<bool> compileDone;
        std::thread compileThread;
        std::mutex outputMutex;
        std::string buildOutput;
    };
}
