# MicroStudio

MicroStudio is a lightweight C++ Integrated Development Environment (IDE) designed to simplify the development process with features like text editing, file management, and code compilation.

## Features

- **Text Editing:** Integrated text editor with syntax highlighting for C++.
- **File Management:** Browse and open files from your project directory.
- **Project Explorer:** Navigate through your project files and directories.
- **Code Compilation:** Compile C++ source files directly from the IDE.
- **Execution:** Run compiled code directly from the IDE.
- **Settings:** Customize the background color of the IDE.

## Picture
<img width="1470" alt="Bildschirmfoto 2024-09-08 um 19 33 18" src="https://github.com/user-attachments/assets/b9d11848-6084-43d8-887f-eebb1d1a9422">


### Build Instructions

To build and run MicroStudio, follow these steps:

1. **Clone the Repository:**
    ```bash
    git clone https://github.com/mendax0110/MicroStudio.git
    ```

2. **Change Directory to the Cloned Repository:**
    ```bash
    cd MicroStudio
    ```

3. **Initialize and Update Submodules:**
    ```bash
    git submodule update --init --recursive
    ```

4. **Create the Build Directory:**
    ```bash
    mkdir build
    ```

5. **Change Directory to the Build Directory:**
    ```bash
    cd build
    ```

6. **Generate CMake Files:**
    ```bash
    cmake ..
    ```

7. **Build the Project:**
    ```bash
    cmake --build .
    ```

### Usage
```bash
./MicroStudio
