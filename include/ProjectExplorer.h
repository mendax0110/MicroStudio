#pragma once

#include "TreeNode.h"
#include <string>
#include <utility>

namespace MicroStudio
{
    class ProjectExplorer
    {
    public:
        static std::pair<std::string, std::pair<std::string, std::pair<std::string, bool>>> DisplayTreeNode(TreeNode& node);
    };
}
