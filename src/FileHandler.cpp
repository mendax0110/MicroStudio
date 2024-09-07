#include "../include/FileHandler.h"

#include <fstream>
#include <iostream>
#include <filesystem>

using namespace MicroStudio;

FileHandler::FileHandler() : fileContentTemplate("// New file template\n\nint main() {\n    return 0;\n}\n")
{

}

FileHandler::~FileHandler() = default;

bool FileHandler::CreateFile(const std::string &filePath, const std::string &fileName)
{
    std::string fullPath = filePath + "/" + fileName;

    if (FileExists(fullPath))
    {
        std::cerr << "[ERROR] File already exists: " << fullPath << std::endl;
        return false;
    }

    std::ofstream newFile(fullPath);
    if (newFile.is_open())
    {
        newFile << fileContentTemplate;
        newFile.close();
        std::cout << "[INFO] File created successfully: " << fullPath << std::endl;
        return true;
    }
    else
    {
        std::cerr << "[ERROR] Failed to create file: " << fullPath << std::endl;
        return false;
    }
}

bool FileHandler::FileExists(const std::string &filePath)
{
    return std::filesystem::exists(filePath);
}