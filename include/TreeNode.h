#pragma once

#include <string>
#include <vector>

namespace MicroStudio
{
    enum class NodeType
    {
        File,
        Directory
    };

    class TreeNode
    {
    public:
        TreeNode(std::string  name, const std::string& basePath, NodeType type);
        ~TreeNode();

        const std::string& GetName() const { return name; }
        const std::string& GetPath() const { return path; }
        NodeType GetType() const { return type; }
        void AddChild(TreeNode* child);
        const std::vector<TreeNode*>& GetChildren() const { return children; }

    private:
        std::string path;
        std::string name;
        NodeType type;
        std::vector<TreeNode*> children;
    };
}
