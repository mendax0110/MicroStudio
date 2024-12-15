cmake_minimum_required(VERSION 3.14)
project(MicroStudio VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (__APPLE__)
        set(GLFW3_INCLUDE_DIRS "/opt/homebrew/opt/glfw/include")
        set(GLFW3_LIBRARIES "/opt/homebrew/opt/glfw/lib/libglfw.dylib")
endif()

if (WIN32)
    set(GLFW3_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/external/dearimgui/examples/libs/glfw/include")
    set(GLFW3_LIBRARIES "${CMAKE_SOURCE_DIR}/external/dearimgui/examples/libs/glfw/lib-vc2010-64/glfw3.lib")
    link_directories("${CMAKE_SOURCE_DIR}/external/dearimgui/examples/libs/glfw/lib-vc2010-64")
endif()

find_package(OpenGL REQUIRED)

include_directories(
        external/dearimgui
        external/imgui_file_dialog
        external/ImGuiColorTextEdit
        external/imgui_club/imgui_multicontext_compositor
        external/imgui_club/imgui_memory_editor
        ${GLFW3_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIRS}
)

set(IMGUI_SOURCES
        external/dearimgui/imgui.cpp
        external/dearimgui/imgui_draw.cpp
        external/dearimgui/imgui_tables.cpp
        external/dearimgui/imgui_widgets.cpp
        external/dearimgui/backends/imgui_impl_glfw.cpp
        external/dearimgui/backends/imgui_impl_opengl3.cpp
)

set(IMGUI_FILE_DIALOG_SOURCES
        external/ImGuiFileDialog/ImGuiFileDialog.cpp
)

set(IMGUI_COLOR_TEXT_EDIT_SOURCES
        external/ImGuiColorTextEdit/TextEditor.cpp
)

set(IMGUI_CLUB
    external/imgui_club/imgui_multicontext_compositor/imgui_multicontext_compositor.cpp
)

set(SOURCES
        src/GuiManager.cpp
        src/TreeNode.cpp
        src/ProjectExplorer.cpp
        src/JsonParser.cpp
        src/CommandExecutor.cpp
        src/CompilerHelper.cpp
        src/DebuggerHelper.cpp
        src/ShellManager.cpp
        src/main.cpp
        ${IMGUI_SOURCES}
        ${IMGUI_FILE_DIALOG_SOURCES}
        ${IMGUI_COLOR_TEXT_EDIT_SOURCES}
        ${IMGUI_CLUB}
        src/FileHandler.cpp
)

add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${GLFW3_LIBRARIES}
        ${OPENGL_LIBRARIES}
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(APPLE)       
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -O3)
endif()

if(WIN32)
        target_compile_options(${PROJECT_NAME} PRIVATE)
endif()