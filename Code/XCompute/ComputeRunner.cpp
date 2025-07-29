// Author: Jake Rieger
// Created: 7/29/2025.
//

#include "ComputeRunner.hpp"
#include "DisplayShaders.hpp"
#include <sstream>
#include <iostream>

namespace x {
    bool ComputeRunner::Init() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        _window = glfwCreateWindow(_windowWidth, _windowHeight, "ComputeRunner", nullptr, nullptr);
        if (!_window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(_window);

        glfwSetWindowUserPointer(_window, this);
        glfwSetKeyCallback(_window, KeyCallback);
        glfwSetFramebufferSizeCallback(_window, FrameBufferSizeCallback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Graphics Card: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

        if (!GLAD_GL_VERSION_4_6) {
            std::cerr << "OpenGL 4.6 not supported (requires 4.3+ for compute shaders)" << std::endl;
            return false;
        }

        GLint maxComputeWorkGroupCount[3];
        GLint maxComputeWorkGroupSize[3];
        GLint maxComputeWorkGroupInvocations;

        for (int idx = 0; idx < 3; ++idx) {
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &maxComputeWorkGroupCount[idx]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &maxComputeWorkGroupSize[idx]);
        }
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxComputeWorkGroupInvocations);

        std::cout << "Max compute work group count: " << maxComputeWorkGroupCount[0] << " x "
                  << maxComputeWorkGroupCount[1] << " x " << maxComputeWorkGroupCount[2] << std::endl;
        std::cout << "Max compute work group size: " << maxComputeWorkGroupSize[0] << " x "
                  << maxComputeWorkGroupSize[1] << " x " << maxComputeWorkGroupSize[2] << std::endl;
        std::cout << "Max compute work group invocations: " << maxComputeWorkGroupInvocations << std::endl;

        SetupDisplayQuad();

        if (!CreateDisplayProgram()) {
            std::cerr << "Failed to create display program" << std::endl;
            return false;
        }

        return true;
    }

    bool ComputeRunner::LoadComputeShader(const fs::path& filename) {
        const std::string shaderSource = ReadFile(filename);
        if (shaderSource.empty()) {
            std::cerr << "Failed to read compute shader file: " << filename << std::endl;
            return false;
        }

        _computeShader     = glCreateShader(GL_COMPUTE_SHADER);
        const char* source = shaderSource.c_str();
        glShaderSource(_computeShader, 1, &source, nullptr);
        glCompileShader(_computeShader);

        GLint success;
        glGetShaderiv(_computeShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(_computeShader, 1024, nullptr, infoLog);
            std::cerr << "Compute shader compilation failed:\n" << infoLog << std::endl;
            return false;
        }

        _computeProgram = glCreateProgram();
        glAttachShader(_computeProgram, _computeShader);
        glLinkProgram(_computeProgram);

        glGetProgramiv(_computeProgram, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetProgramInfoLog(_computeProgram, 1024, nullptr, infoLog);
            std::cerr << "Compute program linking failed:\n" << infoLog << std::endl;
            return false;
        }

        CreateOutputTexture();
        std::cout << "Compute shader loaded successfully: " << filename << std::endl;

        return true;
    }

    int ComputeRunner::Run() const {
        glViewport(0, 0, _windowWidth, _windowHeight);
        glfwSwapInterval(1);  // Enable V-Sync

        double lastTime = glfwGetTime();
        int frameCount  = 0;

        while (!glfwWindowShouldClose(_window)) {
            glfwPollEvents();
            RunComputeShader();
            DisplayResult();
            glfwSwapBuffers(_window);

            const double currentTime = glfwGetTime();
            frameCount++;
            if (currentTime - lastTime >= 1.0) {
                frameCount = 0;
                lastTime   = currentTime;
            }
        }

        return EXIT_SUCCESS;
    }

    void ComputeRunner::KeyCallback(GLFWwindow* window, int key, int, int action, int) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { glfwSetWindowShouldClose(window, GLFW_TRUE); }

        if (key == GLFW_KEY_R && action == GLFW_PRESS) {
            // TODO: Handle reloading shader
            std::cout << "Reload functionality not implemented yet" << std::endl;
        }
    }

    void ComputeRunner::FrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
        auto* runner = static_cast<ComputeRunner*>(glfwGetWindowUserPointer(window));
        if (runner) {
            runner->_windowWidth  = width;
            runner->_windowHeight = height;
            runner->CreateOutputTexture();
        }
    }

    std::string ComputeRunner::ReadFile(const fs::path& filename) {
        const std::ifstream file(filename.string());
        if (!file.is_open()) { return ""; }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void ComputeRunner::CreateOutputTexture() {
        if (_outputTexture != 0) { glDeleteTextures(1, &_outputTexture); }

        glGenTextures(1, &_outputTexture);
        glBindTexture(GL_TEXTURE_2D, _outputTexture);

        // Create texture with RGBA32F format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _windowWidth, _windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindImageTexture(0, _outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void ComputeRunner::SetupDisplayQuad() {
        constexpr float quadVertices[] = {// positions   // texCoords
                                          -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
                                          -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

        glGenVertexArrays(1, &_quadVAO);
        glGenBuffers(1, &_quadVBO);

        glBindVertexArray(_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
    }

    bool ComputeRunner::CreateDisplayProgram() {
        // Create vertex shader
        const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &kVertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        GLint success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
            return false;
        }

        // Create fragment shader
        const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &kFragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
            return false;
        }

        _displayProgram = glCreateProgram();
        glAttachShader(_displayProgram, vertexShader);
        glAttachShader(_displayProgram, fragmentShader);
        glLinkProgram(_displayProgram);

        glGetProgramiv(_displayProgram, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(_displayProgram, 512, nullptr, infoLog);
            std::cerr << "Display program linking failed:\n" << infoLog << std::endl;
            return false;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return true;
    }

    void ComputeRunner::RunComputeShader() const {
        glUseProgram(_computeProgram);

        const GLint timeLocation = glGetUniformLocation(_computeProgram, "uTime");
        if (timeLocation != -1) { glUniform1f(timeLocation, (float)glfwGetTime()); }

        const GLint resolutionLocation = glGetUniformLocation(_computeProgram, "uResolution");
        if (resolutionLocation != -1) { glUniform2f(resolutionLocation, (float)_windowWidth, (float)_windowHeight); }

        const GLint mouseLocation = glGetUniformLocation(_computeProgram, "uMouse");
        if (mouseLocation != -1) {
            double mouseX, mouseY;
            glfwGetCursorPos(_window, &mouseX, &mouseY);
            glUniform2f(mouseLocation, (float)mouseX, (float)mouseY);
        }

        // Dispatch shader
        glDispatchCompute((_windowWidth + 15) / 16, (_windowHeight + 15) / 16, 1);

        // Wait for shader to complete
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void ComputeRunner::DisplayResult() const {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(_displayProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _outputTexture);
        glUniform1i(glGetUniformLocation(_displayProgram, "uOutputTexture"), 0);

        glBindVertexArray(_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void ComputeRunner::Cleanup() const {
        if (_computeProgram) glDeleteProgram(_computeProgram);
        if (_computeShader) glDeleteShader(_computeShader);
        if (_displayProgram) glDeleteProgram(_displayProgram);
        if (_outputTexture) glDeleteTextures(1, &_outputTexture);
        if (_quadVAO) glDeleteVertexArrays(1, &_quadVAO);
        if (_quadVBO) glDeleteBuffers(1, &_quadVBO);
        if (_window) { glfwDestroyWindow(_window); }
        glfwTerminate();
    }
}  // namespace x