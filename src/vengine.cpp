#include <stdexcept>
#include "GLFW/glfw3.h"
#include "vengine.hpp"

inline const int kWidth = 800, kHeight = 600;
inline constexpr const char* kWindowName = "Von Engine";
inline constexpr const char* kEngineName = "Von";

void VEngine::Run() {
    InitWindow();
    Loop();
    Close();
}

void VEngine::InitWindow() {
    if (!glfwInit()) {
        throw std::runtime_error("failed to init GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow(kWidth, kHeight, kWindowName, nullptr, nullptr);
    if (!_window) {
        glfwTerminate();
        throw std::runtime_error("failed to create window!");
    }
}

void VEngine::Loop() {
    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
    }
}

void VEngine::Close() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}
