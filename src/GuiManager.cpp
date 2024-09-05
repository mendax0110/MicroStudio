#include "../include/GuiManager.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <thread>
#include <mutex>
#include <memory>
#include <array>
#include <filesystem>
#include "../include/json.hpp"
#include "../external/dearimgui/examples/libs/glfw/include/GLFW/glfw3.h"

using namespace MicroStudio;
using json = nlohmann::json;

GuiManager::GuiManager()
        : compileDone(false), settingsOpen(false), rootNode(nullptr), bgColor(0.45f, 0.55f, 0.60f, 1.00f), isNewFileLoaded(false)
{
    glfwInit();
    window = glfwCreateWindow(1280, 720, "MicroStudio", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    std::string iniFilePath = "../initialize/imgui.ini";
    io.IniFilename = iniFilePath.c_str();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 120");

    std::ifstream file("project.json");
    if (file.is_open())
    {
        json fileTreeJson;
        file >> fileTreeJson;
        rootNode = new TreeNode("", "", NodeType::Directory);
        JsonParser::Parse(*rootNode, fileTreeJson["root"], fileTreeJson["project"]);
    }

    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    debugger.Initialize();
}

GuiManager::~GuiManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    delete rootNode;
    if (compileThread.joinable())
    {
        compileThread.join();
    }
    debugger.StopDebugging();
}

void GuiManager::Run()
{
    shellManager.StartShellProcess();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderDockingSpace();
        RenderMainMenu();
        RenderProjectExplorer();
        RenderTextEditor();
        RenderOutputWindow();
        if (settingsOpen)
        {
            RenderSettingsWindow();
        }
        RenderFileDialog();

        shellManager.RenderShellWindow();

        if (compileThread.joinable() && compileDone.load())
        {
            compileThread.join();
            ParseBuildOutput();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

void GuiManager::RenderDockingSpace()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();
}

void GuiManager::RenderMainMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Ctrl+O"))
            {
                fileDialog.OpenDialog("ChooseFileDlg", "Choose File", ".cpp,.h,.hpp");
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                if (!filePathName.empty())
                {
                    text = editor.GetText();
                    std::ofstream ofs(filePath + "/" + filePathName, std::ofstream::trunc);
                    if (ofs.is_open())
                    {
                        ofs << text;
                        ofs.close();
                        std::cout << "[INFO] File saved successfully: " << filePathName << std::endl;
                    }
                    else
                    {
                        std::cerr << "[ERROR] Failed to open file for saving: " << filePathName << std::endl;
                    }
                }
                else
                {
                    std::cerr << "[ERROR] No file path name available for saving." << std::endl;
                }
            }
            if (ImGui::MenuItem("Compile", "F5"))
            {
                if (filePathName.empty())
                {
                    ImGui::OpenPopup("No File Selected");
                    std::cerr << "[ERROR] No file selected for compilation." << std::endl;
                    return;
                }

                if (filePathName.substr(filePathName.find_last_of('.') + 1) != "cpp")
                {
                    ImGui::OpenPopup("Invalid File Type");
                    std::cerr << "[ERROR] Invalid file type selected: " << filePathName << std::endl;
                    return;
                }

                compileDone.store(false);
                buildOutput.clear();
                output.clear();

                std::string cmakeListsContent = R"(
                    cmake_minimum_required(VERSION 3.10)
                    project(DynamicBuild)

                    set(CMAKE_CXX_STANDARD 17)

                    add_executable(DynamicBuild )" + filePath + "/" + filePathName + R"()
                )";

                std::ofstream cmakeListsFile("CMakeLists.txt");
                if (cmakeListsFile.is_open())
                {
                    cmakeListsFile << cmakeListsContent;
                    cmakeListsFile.close();
                    std::cout << "[INFO] Generated CMakeLists.txt with content:\n" << cmakeListsContent << std::endl;
                }
                else
                {
                    std::cerr << "[ERROR] Failed to open CMakeLists.txt for writing." << std::endl;
                    return;
                }

                std::string cleanBuildCommand = "rm -rf build";
                std::string buildCommand = "cmake -S . -B build && cmake --build build --config Release";

                std::cout << "[INFO] Running clean command: " << cleanBuildCommand << std::endl;
                std::cout << "[INFO] Running build command: " << buildCommand << std::endl;

                compileThread = std::thread([this, cleanBuildCommand, buildCommand]() {
                    std::array<char, 128> buffer{};
                    std::string result;

                    std::unique_ptr<FILE, decltype(&pclose)> cleanPipe(popen(cleanBuildCommand.c_str(), "r"), pclose);
                    if (cleanPipe)
                    {
                        while (fgets(buffer.data(), buffer.size(), cleanPipe.get()) != nullptr)
                        {
                            result += buffer.data();
                        }
                        std::cout << "[INFO] Clean command output:\n" << result << std::endl;
                    }
                    else
                    {
                        std::cerr << "[ERROR] Clean command failed: " << cleanBuildCommand << std::endl;
                    }

                    result.clear();

                    std::unique_ptr<FILE, decltype(&pclose)> buildPipe(popen(buildCommand.c_str(), "r"), pclose);
                    if (!buildPipe)
                    {
                        result = "popen() failed!";
                        std::cerr << "[ERROR] Build command failed to start: " << buildCommand << std::endl;
                    }
                    else
                    {
                        while (fgets(buffer.data(), buffer.size(), buildPipe.get()) != nullptr)
                        {
                            result += buffer.data();
                        }
                        std::cout << "[INFO] Build command output:\n" << result << std::endl;
                    }

                    {
                        std::lock_guard<std::mutex> lock(outputMutex);
                        buildOutput = result;
                        output = result;
                    }
                    compileDone.store(true);
                });
            }
            if (ImGui::MenuItem("Run", "Ctrl+R"))
            {
                std::cout << "[INFO] Running compiled code." << std::endl;
                RunCompiledCode();
            }
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                std::cout << "[INFO] Exiting application." << std::endl;
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, undoStack.size() > 1))
            {
                redoStack.push(text);
                undoStack.pop();
                text = undoStack.top();
                std::cout << "[INFO] Undo operation performed." << std::endl;
            }

            if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, !redoStack.empty()))
            {
                undoStack.push(text);
                text = redoStack.top();
                redoStack.pop();
                std::cout << "[INFO] Redo operation performed." << std::endl;
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Open Settings"))
            {
                settingsOpen = true;
                std::cout << "[INFO] Settings menu opened." << std::endl;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (ImGui::BeginPopup("No File Selected"))
    {
        ImGui::Text("No file selected for compilation.");
        if (ImGui::Button("OK"))
        {
            std::cout << "[INFO] No file selected popup closed." << std::endl;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Invalid File Type"))
    {
        ImGui::Text("Selected file is not a .cpp file. Please select a valid .cpp file to compile.");
        if (ImGui::Button("OK"))
        {
            std::cout << "[INFO] Invalid file type popup closed." << std::endl;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("No Executable"))
    {
        ImGui::Text("No executable set. Please compile your code first.");
        if (ImGui::Button("OK"))
        {
            std::cout << "[INFO] No executable popup closed." << std::endl;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void GuiManager::RenderFileDialog()
{
    if (fileDialog.Display("ChooseFileDlg"))
    {
        if (fileDialog.IsOk())
        {
            std::filesystem::path selectedPath = fileDialog.GetFilePathName();
            std::string fileName = selectedPath.filename().string();
            std::string filePath = selectedPath.parent_path().string();

            filePathName = fileName;
            rootDirectory = filePath;

            bool isFileOpen = false;
            for (size_t i = 0; i < openFiles.size(); ++i)
            {
                if (openFiles[i].filePath == filePath && openFiles[i].fileName == fileName)
                {
                    currentFileIndex = i;
                    isFileOpen = true;
                    break;
                }
            }

            if (!isFileOpen)
            {
                OpenFile newFile;
                newFile.fileName = fileName;
                newFile.filePath = filePath;

                std::ifstream inFile(selectedPath, std::ios::in | std::ios::binary);
                if (inFile)
                {
                    std::ostringstream ss;
                    ss << inFile.rdbuf();
                    newFile.content = ss.str();

                    newFile.editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
                    newFile.editor.SetText(newFile.content);

                    openFiles.push_back(newFile);
                    currentFileIndex = openFiles.size() - 1;
                }
                else
                {
                    std::cerr << "Failed to open file: " << selectedPath << std::endl;
                }
            }
        }
        fileDialog.Close();
    }
}

void GuiManager::RenderProjectExplorer()
{
    if (!rootDirectory.empty())
    {
        ImGui::Begin("Project Explorer");

        std::function<void(const std::filesystem::path&)> renderDirectory = [&](const std::filesystem::path& directory)
        {
            for (const auto& entry : std::filesystem::directory_iterator(directory))
            {
                const std::string& path = entry.path().string();
                const bool isDirectory = entry.is_directory();
                const bool selected = (currentFileIndex != -1) && (openFiles[currentFileIndex].filePath == path) && (openFiles[currentFileIndex].fileName == entry.path().filename().string());

                if (isDirectory)
                {
                    if (ImGui::TreeNode(entry.path().filename().string().c_str()))
                    {
                        renderDirectory(entry.path());
                        ImGui::TreePop();
                    }
                }
                else
                {
                    if (ImGui::Selectable(entry.path().filename().string().c_str(), selected))
                    {
                        bool isFileOpen = false;
                        for (size_t i = 0; i < openFiles.size(); ++i)
                        {
                            if (openFiles[i].filePath == path && openFiles[i].fileName == entry.path().filename().string())
                            {
                                isFileOpen = true;
                                currentFileIndex = i;
                                break;
                            }
                        }

                        if (!isFileOpen)
                        {
                            OpenFile newFile;
                            newFile.fileName = entry.path().filename().string();
                            newFile.filePath = path;

                            std::ifstream inFile(entry.path(), std::ios::in | std::ios::binary);
                            if (inFile)
                            {
                                std::ostringstream ss;
                                ss << inFile.rdbuf();
                                newFile.content = ss.str();

                                newFile.editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
                                newFile.editor.SetText(newFile.content);

                                openFiles.push_back(newFile);
                                currentFileIndex = openFiles.size() - 1;
                            }
                            else
                            {
                                std::cerr << "Failed to open file: " << entry.path() << std::endl;
                            }
                        }
                    }
                }
            }
        };

        renderDirectory(rootDirectory);

        ImGui::End();
    }
}

void GuiManager::RenderTextEditor()
{
    if (openFiles.empty()) return;

    ImGui::Begin("Text Editor");

    if (ImGui::BeginTabBar("OpenFiles"))
    {
        for (size_t i = 0; i < openFiles.size(); ++i)
        {
            bool isOpen = true;
            if (ImGui::BeginTabItem(openFiles[i].fileName.c_str(), &isOpen))
            {
                currentFileIndex = i;

                filePathName = openFiles[i].fileName;
                filePath = openFiles[i].filePath;

                if (openFiles[i].editor.IsTextChanged())
                {
                    openFiles[i].content = openFiles[i].editor.GetText();
                }

                openFiles[i].editor.Render(openFiles[i].fileName.c_str());

                ImGui::EndTabItem();
            }

            if (!isOpen)
            {
                openFiles.erase(openFiles.begin() + i);
                if (static_cast<size_t>(currentFileIndex) >= i)
                {
                    currentFileIndex = std::max(0, currentFileIndex - 1);
                }
                --i;
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void GuiManager::RenderOutputWindow()
{
    ImGui::Begin("Output");

    std::lock_guard<std::mutex> lock(outputMutex);
    ImGui::TextUnformatted(output.c_str());

    ImGui::End();
}

void GuiManager::RenderSettingsWindow()
{
    ImGui::Begin("Settings", &settingsOpen);
    bgColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGui::ColorEdit3("Background Color", (float*)&bgColor);
    ImGui::End();
}

void GuiManager::ParseBuildOutput()
{
    std::string line;
    std::istringstream outputStream(buildOutput);

    while (std::getline(outputStream, line))
    {
        if (line.find("Built target") != std::string::npos)
        {
            std::size_t pos = line.find_last_of(" ");
            if (pos != std::string::npos)
            {
                executableName = line.substr(pos + 1);
            }
        }
    }
}

void GuiManager::RunCompiledCode()
{
    if (filePathName.empty())
    {
        ImGui::OpenPopup("No Executable");
        return;
    }

    std::string baseName = filePathName.substr(0, filePathName.find_last_of('.'));
    std::string executableName = baseName;

    std::string command = "./build/" + executableName;

    std::thread runThread([command]() {
        system(command.c_str());
    });
    runThread.detach();
}

void GuiManager::CompileSelectedFile()
{
    if (filePathName.empty()) {
        std::cerr << "No file selected for compilation." << std::endl;
        return;
    }

    if (filePathName.substr(filePathName.find_last_of('.') + 1) != "cpp")
    {
        std::cerr << "Selected file is not a .cpp file." << std::endl;
        return;
    }

    std::string baseName = filePathName.substr(0, filePathName.find_last_of('.'));
    std::string executableName = baseName + "_exe";

    std::string compileCommand = "g++ -o build/" + executableName + " " + filePath;

    std::cout << "Compile command: " << compileCommand << std::endl;

    compileDone.store(false);
    buildOutput.clear();

    compileThread = std::thread([this, compileCommand]() {
        std::array<char, 128> buffer{};
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(compileCommand.c_str(), "r"), pclose);
        if (!pipe)
        {
            result = "popen() failed!";
        }
        else
        {
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            {
                result += buffer.data();
            }
        }
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            buildOutput = result;
            output = result;
        }
        compileDone.store(true);
    });
}

void GuiManager::OpenFolder()
{

}
