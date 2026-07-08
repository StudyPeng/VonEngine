#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

inline constexpr uint32_t k_width = 800, k_height = 600;
inline constexpr const char* k_wnd_name = "Von Engine";

class TriangleApp {
 public:
  void run() {
    init_window();
    loop();
    cleanup();
  }

 private:
  GLFWwindow* wnd = nullptr;

  void init_window() {
    if (!glfwInit()) {
      throw std::runtime_error("failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    wnd = glfwCreateWindow(k_width, k_height, k_wnd_name, nullptr, nullptr);
    if (!wnd) {
      glfwTerminate();
      throw std::runtime_error("failed to create window");
    }
  }

  void init_vulkan() {}

  void loop() {
    while (!glfwWindowShouldClose(wnd)) {
      glfwPollEvents();
    }
  }

  void cleanup() {
    glfwDestroyWindow(wnd);
    glfwTerminate();
  }
};

int main() {
  TriangleApp app;
  try {
    app.run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}