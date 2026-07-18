#pragma once
#include "GLFW/glfw3.h"

class VEngine {
   public:
    void Run();
    void InitWindow();
    void InitVulkan();
    void Loop();
    void Close();

   private:
    GLFWwindow* _window;
};