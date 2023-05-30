
#include <filesystem>

#include "Editor.h"

auto main(int /*argc*/, char* /*argv*/[]) -> int
{
    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    exage::init();
    exitor::Editor editor;
    editor.run();

    return 0;
}