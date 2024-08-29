#include "../include/JsonParser.h"
#include <iostream>
#include <filesystem>

using namespace MicroStudio;

void JsonParser::Parse(TreeNode& treeNode, const std::string& basePath, const std::string& projectName, bool setProjectName)
{
    std::filesystem::path path(basePath);
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
    {
        treeNode = TreeNode(path.filename().string(), basePath, NodeType::Directory);
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            NodeType type = entry.is_directory() ? NodeType::Directory : NodeType::File;
            auto* childNode = new TreeNode(entry.path().filename().string(), entry.path().string(), type);
            treeNode.AddChild(childNode);
            if (type == NodeType::Directory)
            {
                Parse(*childNode, entry.path().string(), projectName, false);
            }
        }
    }
    else
    {
        std::cerr << "Invalid directory path: " << basePath << std::endl;
    }
    
    if (setProjectName)
    {
        treeNode = TreeNode(treeNode.GetName(), basePath, NodeType::Directory);
    }
}
