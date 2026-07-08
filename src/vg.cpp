#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

inline constexpr uint32_t kWidth = 800, k_height = 600;
inline constexpr const char* kAppName = "Von Engine";

class TriangleApp {
 public:
  void run() {
    init_window();
    init_vulkan();
    loop();
    cleanup();
  }

 private:
  GLFWwindow* wnd = nullptr;
  VkInstance instance = nullptr;

  void init_window() {
    if (!glfwInit()) {
      throw std::runtime_error("failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    wnd = glfwCreateWindow(kWidth, k_height, kAppName, nullptr, nullptr);
    if (!wnd) {
      glfwTerminate();
      throw std::runtime_error("failed to create window");
    }
  }

  void init_vulkan() {
    create_instance();
  }

  void loop() {
    while (!glfwWindowShouldClose(wnd)) {
      glfwPollEvents();
    }
  }

  void cleanup() {
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(wnd);
    glfwTerminate();
  }

  void create_instance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = kAppName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
      std::cout << "(" << i + 1 << ") " << glfwExtensions[i] << "\n";
    }

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;  // Not define right now ^^

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    int32_t loopCount = 0;
    std::cout << "\n" << "available extensions:\n";
    for (const auto& extension : extensions) {
      ++loopCount;
      std::cout << "(" << loopCount << ") " << extension.extensionName << "\n";
    }
  }

  bool compare_extension(std::vector<VkExtensionProperties>& vkExtensions, char** glfwExtensions) {
    return false;
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