#include "present.hpp"

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

        uint32_t device_extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr,
                                             &device_extension_count, nullptr);

        std::vector<VkExtensionProperties> device_extensions(
            device_extension_count);
        vkEnumerateDeviceExtensionProperties(
            device, nullptr, &device_extension_count, device_extensions.data());

        bool has_swapchain_support = false;

        for (const auto &extension : device_extensions) {
            if (std::string_view{extension.extensionName} ==
                std::string_view{VK_KHR_SWAPCHAIN_EXTENSION_NAME}) {
                has_swapchain_support = true;
            }
        }

        if (graphics_family.has_value() && present_family.has_value() &&
            has_swapchain_support) {
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

    const std::array extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const VkDeviceCreateInfo device_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount =
            static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = extensions.size(),
        .ppEnabledExtensionNames = extensions.data(),
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

void Device::submit_to_graphics(VkCommandBuffer command_buffer,
                                const Semaphore &wait_semaphore,
                                const Semaphore &signal_semaphore,
                                const Fence &fence) const {

    const auto wait_semaphore_raw = wait_semaphore.get();
    const auto signal_semaphore_raw = signal_semaphore.get();

    const VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &wait_semaphore_raw,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &signal_semaphore_raw,
    };

    VK_ERROR(vkQueueSubmit(graphics_queue, 1, &submit_info, fence.get()));
}

void Device::present(const Swapchain &swapchain,
                     const Semaphore &wait_semaphore,
                     uint32_t image_index) const {

    const auto wait_semaphore_raw = wait_semaphore.get();
    const auto swapchain_raw = swapchain.get();

    const VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &wait_semaphore_raw,
        .swapchainCount = 1,
        .pSwapchains = &swapchain_raw,
        .pImageIndices = &image_index,
        .pResults = nullptr,
    };

    VK_ERROR(vkQueuePresentKHR(present_queue, &present_info));
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

CommandPool::CommandPool(const Device &device) : device(device) {
    const VkCommandPoolCreateInfo pool_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = device.get_graphics_family(),
    };

    VK_ERROR(vkCreateCommandPool(device.get(), &pool_info, nullptr, &pool));
}

auto CommandPool::allocate_buffer() const -> VkCommandBuffer {
    VkCommandBufferAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer buffer;
    VK_ERROR(vkAllocateCommandBuffers(device.get(), &alloc_info, &buffer));
    return buffer;
}
