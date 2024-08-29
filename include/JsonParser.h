#pragma once

#include "TreeNode.h"
#include <string>

namespace MicroStudio
{
    class JsonParser
    {
    public:
        static void Parse(TreeNode& treeNode, const std::string& basePath, const std::string& projectName, bool setProjectName = true);
    };
}
