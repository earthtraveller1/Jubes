#include "devices.hpp"

namespace {
constexpr std::string_view VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";

bool check_validation_layers_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

    for (const auto &layer : layers) {
        if (VALIDATION_LAYER == std::string_view{layer.layerName}) {
            return true;
        }
    }

    return false;
}
} // namespace

std::tuple<Device, Error> Device::create(GLFWwindow *const p_window,
                                         bool p_enable_validation) {
    VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Jubes",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = nullptr,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_3,
    };

    if (p_enable_validation && !check_validation_layers_support()) {
        return {{}, Error::NoValidationLayers};
    }

    const char *validation_layer = VALIDATION_LAYER.data();

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo instance_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = p_enable_validation ? 1 : 0,
        .ppEnabledLayerNames =
            p_enable_validation ? &validation_layer : nullptr,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions,
    };

    VkInstance instance;
    auto result = vkCreateInstance(&instance_info, nullptr, &instance);
    if (result != VK_SUCCESS) {
        fmt::println("[ERROR]: Failed to create the instance: {}", result);
        return {{}, Error::VulkanError};
    }

    return {
        {
            instance,
        },
        Error::Ok,
    };
}