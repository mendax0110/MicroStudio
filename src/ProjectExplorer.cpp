#include "../include/ProjectExplorer.h"
#include <imgui.h>
#include <iostream>
#include <fstream>

using namespace MicroStudio;

std::pair<std::string, std::pair<std::string, std::pair<std::string, bool>>> ProjectExplorer::DisplayTreeNode(TreeNode& node)
{
    std::string content;
    std::string filePath;
    std::string fileName;
    bool fileSelected = false;

    if (node.GetType() == NodeType::Directory)
    {
        if (ImGui::TreeNode(node.GetName().c_str()))
        {
            for (auto& child : node.GetChildren())
            {
                auto result = DisplayTreeNode(*child);
                content += result.first;
                fileSelected = result.second.second.second;
                if (fileSelected)
                {
                    filePath = result.second.first;
                    fileName = result.second.second.first;
                }
            }
            ImGui::TreePop();
        }
    }
    else
    {
        if (ImGui::Selectable(node.GetName().c_str()))
        {
            fileName = node.GetName();
            filePath = node.GetPath();
            std::ifstream file(filePath + "\\" + fileName);
            std::string line;
            while (std::getline(file, line))
            {
                content += line + "\n";
            }
            fileSelected = true;
        }
    }

    return { content, { filePath, { fileName, fileSelected } } };
}
