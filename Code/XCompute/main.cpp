#pragma warning(disable : 4100)

#include "ComputeRunner.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::string exeName    = argv[0];
        const size_t lastSlash = exeName.find_last_of("/\\");
        if (lastSlash != std::string::npos) { exeName = exeName.substr(lastSlash + 1); }

        std::cerr << "Usage: " << exeName << " <compute_shader>" << std::endl;
        std::cerr << "Example: " << exeName << " Shaders/mandelbrot.glsl" << std::endl;

        return EXIT_FAILURE;
    }

    x::ComputeRunner computeRunner(1280, 720);

    if (!computeRunner.Init()) {
        std::cerr << "Failed to initialize compute shader runner" << std::endl;
        return EXIT_FAILURE;
    }

    if (!computeRunner.LoadComputeShader(argv[1])) {
        std::cerr << "Failed to initialize compute shader runner" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Running compute shader. Press ESC to exit, R to reload." << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC - Exit application" << std::endl;
    std::cout << "  R   - Reload shader (if implemented)" << std::endl;
    std::cout << "  Mouse - Interactive input (if shader supports uMouse uniform)" << std::endl;

    return computeRunner.Run();
}