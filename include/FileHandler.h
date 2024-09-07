#pragma once

#include <string>

namespace MicroStudio
{
    class FileHandler
    {
    public:
        FileHandler();
        ~FileHandler();

        bool CreateFile(const std::string& filePath, const std::string& fileName);
        static bool FileExists(const std::string& filePath);

    private:
        std::string fileContentTemplate;
    };
}