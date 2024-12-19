#include "devices.hpp"

namespace {
struct PhysicalDevice {
    VkPhysicalDevice physical_device;
    uint32_t graphics_family;
    uint32_t present_family;
};

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

std::optional<PhysicalDevice> pick_physical_device(VkInstance p_instance,
                                                   VkSurfaceKHR p_surface) {
    uint32_t device_count;
    vkEnumeratePhysicalDevices(p_instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(p_instance, &device_count, devices.data());

    for (const auto device : devices) {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                                 nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                                 queue_families.data());

        for (uint32_t i = 0; i < queue_families.size(); i++) {
            if (queue_families.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphics_family = i;
            }

            VkBool32 supports_presentation;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, p_surface,
                                                 &supports_presentation);
            if (supports_presentation) {
                present_family = i;
            }

            // No point in continuing if the two things have already been found.
            if (graphics_family.has_value() && present_family.has_value()) {
                break;
            }
        }

        if (graphics_family.has_value() && present_family.has_value()) {
            return PhysicalDevice{device, graphics_family.value(),
                                  present_family.value()};
        }
    }

    return {};
}
} // namespace

Device::Device(GLFWwindow *const p_window, bool p_enable_validation) {
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
        throw Error::NoValidationLayers;
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
        .enabledLayerCount = p_enable_validation ? (uint32_t)1 : (uint32_t)0,
        .ppEnabledLayerNames =
            p_enable_validation ? &validation_layer : nullptr,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions,
    };

    auto result = vkCreateInstance(&instance_info, nullptr, &instance);
    if (result != VK_SUCCESS) {
        fmt::println("[ERROR]: Failed to create the instance: {}", result);
        throw Error::VulkanError;
    }

    result = glfwCreateWindowSurface(instance, p_window, nullptr, &surface);
    if (result != VK_SUCCESS) {
        fmt::println("[ERROR]: Failed to create the window surface: {}",
                     result);
        throw Error::VulkanError;
    }

    const auto physical_device_stuff = pick_physical_device(instance, surface);
    if (!physical_device_stuff.has_value()) {
        fmt::println("[ERROR]: Could not find an adequate physical device.");
        throw Error::NoAdequatePhysicalDeviceError;
    }

    const auto [physical_device, graphics_family, present_family] =
        physical_device_stuff.value();

    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
    fmt::println("[INFO]: Selected {} as the physical device.",
                 physical_device_properties.deviceName);

    this->physical_device = physical_device;
    this->graphics_family = graphics_family;
    this->present_family = present_family;

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.reserve(2);

    float queue_priority = 1.0f;

    if (graphics_family == present_family) {
        queue_create_infos.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = graphics_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        });
    } else {
        queue_create_infos.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = graphics_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        });

        queue_create_infos.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = present_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        });
    }

    const VkPhysicalDeviceFeatures device_features{};

    const VkDeviceCreateInfo device_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures = &device_features,
    };

    result = vkCreateDevice(physical_device, &device_info, nullptr, &device);
    if (result != VK_SUCCESS) {
        fmt::println("[ERROR]: Failed to create the Vulkan device: {}", result);
        throw Error::VulkanError;
    }

    vkGetDeviceQueue(device, graphics_family, 0, &graphics_queue);
    vkGetDeviceQueue(device, present_family, 0, &present_queue);
}

Device &Device::operator=(Device &&rhs) noexcept {
    instance = rhs.instance;
    surface = rhs.surface;
    physical_device = rhs.physical_device;
    device = rhs.device;
    graphics_family = rhs.graphics_family;
    present_family = rhs.present_family;
    graphics_queue = rhs.graphics_queue;
    present_queue = rhs.present_queue;

    rhs.instance = 0;
    rhs.surface = 0;
    rhs.physical_device = 0;
    rhs.device = 0;
    rhs.graphics_family = 0;
    rhs.present_family = 0;
    rhs.graphics_queue = 0;
    rhs.present_queue = 0;

    return *this;
}

Device::~Device() {
    if (device != VK_NULL_HANDLE) {
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
}
