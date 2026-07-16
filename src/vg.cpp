#include <cstdint>
#include <cstring>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <optional>

inline constexpr uint32_t kWidth = 800, k_height = 600;
inline constexpr const char* kAppName = "Von Engine";
inline constexpr const char* kEngineName = "Von";
inline const std::vector<const char*> kValidationLayers = {"VK_LAYER_KHRONOS_validation"};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isCompelete() {
        return graphicsFamily.has_value();
    }
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class EngineApp {
   public:
    void run() {
        InitWindow();
        InitVulkan();
        Loop();
        Cleanup();
    }

   private:
    GLFWwindow* _wnd = nullptr;
    VkInstance _instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VkQueue _graphicsQueue;

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
        SetupDebugMessenger();
        PickPhysicalDevice();
        CreateLogicDevice();
    }

    void Loop() {
        while (!glfwWindowShouldClose(_wnd)) {
            glfwPollEvents();
        }
    }

    void Cleanup() {
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        }
        vkDestroyDevice(_device, nullptr);
        vkDestroyInstance(_instance, nullptr);
        glfwDestroyWindow(_wnd);
        glfwTerminate();
    }

    void CreateInstance() {
        if (enableValidationLayers && !CheckValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = kAppName;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = kEngineName;
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        std::vector<const char*> glfwExtensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
        createInfo.ppEnabledExtensionNames = glfwExtensions.data();
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
            createInfo.ppEnabledLayerNames = kValidationLayers.data();
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void PickPhysicalDevice() {
        _physicalDevice = VK_NULL_HANDLE;
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
        std::cout << "Device count: " << deviceCount << std::endl;
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());
        for (const auto& device : devices) {
            if (IsDeviceSuitable(device)) {
                _physicalDevice = device;
                break;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        std::multimap<int, VkPhysicalDevice> candidates;
        for (const auto& device : devices) {
            int score = RateDeviceSuitability(device);
            candidates.emplace(score, device);
        }

        if (candidates.rbegin()->first > 0) {
            _physicalDevice = candidates.rbegin()->second;
        } else {
            throw std::runtime_error("failed to find a runtime GPU!");
        }
    }

    void CreateLogicDevice() {
        QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;
        createInfo.enabledLayerCount = 0;

        if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
    }

    int RateDeviceSuitability(VkPhysicalDevice device) {
        int score = 0;
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }
        score += deviceProperties.limits.maxImageDimension2D;
        if (!deviceFeatures.geometryShader) {
            return 0;
        }
        return score;
    }

    bool IsDeviceSuitable(VkPhysicalDevice physicalDevice) {
        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        return indices.isCompelete();
    }

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;
#pragma region Debug Log
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Graphics" << std::endl;
            } else if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Compute" << std::endl;
            } else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Transfer" << std::endl;
            } else if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Binding" << std::endl;
            } else if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Protected" << std::endl;
            } else if (queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Video decode khr" << std::endl;
            } else if (queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Video encode khr" << std::endl;
            } else if (queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Flow" << std::endl;
            } else if (queueFamily.queueFlags & VK_QUEUE_DATA_GRAPH_BIT_ARM) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Data graph arm" << std::endl;
            } else if (queueFamily.queueFlags & VK_QUEUE_FLAG_BITS_MAX_ENUM) {
                std::cout << "QFamily: " << queueFamily.queueFlags << "(" << queueFamily.queueCount
                          << ") " << "Flag Max" << std::endl;
            }
        }

#pragma endregion
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            if (indices.isCompelete()) {
                break;
            }
            i++;
        }
        return indices;
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

    bool CheckValidationLayerSupport() {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        bool found = false;

        for (const char* layerName : kValidationLayers) {
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    found = true;
                    break;
                }
            }
        }

        return found;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageServerity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData) {
        std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }

    void SetupDebugMessenger() {
        if (!enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);
        if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
};

int main() {
    EngineApp app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}