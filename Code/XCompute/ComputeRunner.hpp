// Author: Jake Rieger
// Created: 7/29/2025.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <filesystem>

namespace x {
    namespace fs = std::filesystem;

    class ComputeRunner {
    private:
        GLFWwindow* _window {nullptr};
        GLuint _computeShader {0};
        GLuint _computeProgram {0};
        GLuint _outputTexture {0};
        GLuint _quadVAO {0};
        GLuint _quadVBO {0};
        GLuint _displayProgram {0};
        int _windowWidth;
        int _windowHeight;

    public:
        explicit ComputeRunner(int width = 1280, int height = 720) : _windowWidth(width), _windowHeight(height) {}

        ~ComputeRunner() {
            Cleanup();
        }

        bool Init();
        bool LoadComputeShader(const fs::path& filename);
        int Run() const;

    private:
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
        static std::string ReadFile(const fs::path& filename);

        void CreateOutputTexture();
        void SetupDisplayQuad();
        bool CreateDisplayProgram();
        void RunComputeShader() const;
        void DisplayResult() const;
        void Cleanup() const;
    };
}  // namespace x
