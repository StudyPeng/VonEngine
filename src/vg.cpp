#include <cstdint>
#include <cstring>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

inline constexpr uint32_t kWidth = 800, k_height = 600;
inline constexpr const char* kAppName = "Von Engine";
inline constexpr const char* kEngineName = "Von";
inline const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class TriangleApp {
 public:
  void run() {
    InitWindow();
    InitVulkan();
    Loop();
    Cleanup();
  }

 private:
  GLFWwindow* _wnd = nullptr;
  VkInstance _instance = nullptr;

  void InitWindow() {
    if (!glfwInit()) {
      throw std::runtime_error("failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _wnd = glfwCreateWindow(kWidth, k_height, kAppName, nullptr, nullptr);
    if (!_wnd) {
      glfwTerminate();
      throw std::runtime_error("failed to create window");
    }
  }

  void InitVulkan() {
    CreateInstance();
  }

  void Loop() {
    while (!glfwWindowShouldClose(_wnd)) {
      glfwPollEvents();
    }
  }

  void Cleanup() {
    vkDestroyInstance(_instance, nullptr);
    glfwDestroyWindow(_wnd);
    glfwTerminate();
  }

  void CreateInstance() {
    if (enableValidationLayers && CheckValidationLayerSupport()) {
      throw std::runtime_error("validation layers requested, but not available!");
    }

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
    std::vector<const char*> glfwExtensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions.data();
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }

    // uint32_t extensionCount = 0;
    // vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    // std::vector<VkExtensionProperties> extensions(extensionCount);
    // vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
  }

  bool CheckValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    bool found = false;
    for (const char* layerName : validationLayers) {
      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName)) {
          found = true;
          break;
        }
      }
    }

    if (!found) {
      return false;
    }

    return false;
  }

  std::vector<const char*> GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
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