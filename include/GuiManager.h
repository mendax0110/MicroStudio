#pragma once

#include <string>
#include <atomic>
#include <imgui.h>
#include "../external/dearimgui/backends/imgui_impl_glfw.h"
#include "../external/dearimgui/backends/imgui_impl_opengl3.h"
#include "ProjectExplorer.h"
#include "JsonParser.h"
#include "CommandExecutor.h"
#include "CompilerHelper.h"
#include "DebuggerHelper.h"
#include "ShellManager.h"
#include <thread>
#include <stack>
#include <mutex>
#include "../external/ImGuiFileDialog/ImGuiFileDialog.h"
#include "../external/ImGuiColorTextEdit/TextEditor.h"

namespace MicroStudio
{
    class GuiManager
    {
    public:
        GuiManager();
        ~GuiManager();
        void Run();

    private:
        void RenderMainMenu();
        void RenderFileDialog();
        void RenderProjectExplorer();
        void RenderTextEditor();
        void RenderOutputWindow();
        void RenderSettingsWindow();
        static void RenderDockingSpace();
        void ParseBuildOutput();
        void RunCompiledCode();
        void CompileSelectedFile();
        void OpenFolder();

        struct OpenFile
        {
            std::string fileName;
            std::string filePath;
            std::string content;
            TextEditor editor;
        };

        std::vector<OpenFile> openFiles;
        int currentFileIndex = -1;

        GLFWwindow* window;
        ImGuiFileDialog fileDialog;
        std::thread compileThread;
        std::atomic<bool> compileDone;
        std::string filePathName;
        std::string filePath;
        std::string text;
        std::string output;
        bool settingsOpen;
        std::stack<std::string> undoStack;
        std::stack<std::string> redoStack;
        TreeNode* rootNode;
        ImVec4 bgColor;
        int windowWidth{};
        int windowHeight{};
        std::mutex outputMutex;
        std::string executableName;
        std::string buildOutput;
        std::unique_ptr<CompilerHelper> compilerHelper;
        TextEditor editor;
        bool isNewFileLoaded{};
        std::string rootDirectory;
        DebuggerHelper debugger;
        ShellManager shellManager;
    };
}
