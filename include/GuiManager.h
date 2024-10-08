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
#include "FileHandler.h"
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

        void LoadData(const std::vector<uint8_t>& newData);
        void UpdateData(const std::vector<uint8_t>& newData);

        CompilerType currentCompiler = CompilerType::SingleFile;
        CompilerTarget currentTarget = CompilerTarget::SingleFile;

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
        void MemoryEditor();
        void OpenFileAndLoadData(const std::filesystem::path& path);
        static void SetupDocking();
        void Redo();
        void Undo();
        void RunBuildCommands();
        void RunBuildCommands(const std::string& buildSystem, CompilerTarget target);
        void GenerateCMakeLists();
        void GenerateCMakeListsForTarget(CompilerTarget target);
        void CompileFile();
        void CompileSingleFile();
        void SaveFile();
        static void RenderPopups();
        static void RenderHelpMenu();
        void RenderSettingsMenu();
        void RenderEditMenu();
        void RenderFileMenu();
        void RenderMainMenuBar();
        void HandleFileOpen(const std::string& fileName,
                            const std::string& filePath,
                            const std::filesystem::path& selectedPath);
        void HandleOpenFileDialog();
        bool IsFileOpen(const std::string& filePath, const std::string& fileName);
        void HandleCreateFileDialog();
        void CreateNewFileFromDialog(const char* fileNameBuffer);
        static std::string GetPlatformSpecificCleanCommand();
        void RunCommand(const std::string& command);
        std::string GenerateProjectFileList();
        void CompileProject();
        std::vector<CompilerError> ParseCompilerErrors(const std::string& buildOutput);
        void ShowErrorList(const std::vector<CompilerError>& errors);

        struct OpenFile
        {
            std::string fileName;
            std::string filePath;
            std::string content;
            TextEditor editor;
            std::string fileData;
        };
        static OpenFile CreateNewFile(const std::string& fileName, const std::string& filePath, const std::filesystem::path& selectedPath);

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
        FileHandler fileHandler;

        std::vector<uint8_t> data;
        size_t data_size;
    };

    enum class ActiveDialog
    {
        None,
        OpenFile,
        CreateFile,
        SaveFile
    };
}
