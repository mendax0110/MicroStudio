#include "../include/GuiManager.h"
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        MicroStudio::GuiManager guiManager;
        guiManager.Run();
    }
    catch (const std::exception& ex) 
    {
        std::cerr << "An error occurred: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
