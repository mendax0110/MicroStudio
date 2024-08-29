#include "../include/TreeNode.h"
#include <filesystem>
#include <utility>

using namespace MicroStudio;

TreeNode::TreeNode(std::string  name, const std::string& basePath, NodeType type)
    : name(std::move(name)), type(type)
{
    path = std::filesystem::current_path().string() + basePath;
}

TreeNode::~TreeNode()
{
    for (auto& child : children)
    {
        delete child;
    }
}

void TreeNode::AddChild(TreeNode* child)
{
    children.push_back(child);
}
