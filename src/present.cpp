#include "present.hpp"

Swapchain::Swapchain(const Device &p_device, GLFWwindow *p_window)
    : device(p_device) {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        p_device.get_physical(), p_device.get_surface(), &surface_capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(p_device.get_physical(),
                                         p_device.get_surface(), &format_count,
                                         nullptr);

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(p_device.get_physical(),
                                              p_device.get_surface(),
                                              &present_mode_count, nullptr);

    // In the part in which we select a device, there should be at least one
    // format and one swapchain, so we should not need to worry about it.

    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(p_device.get_physical(),
                                         p_device.get_surface(), &format_count,
                                         formats.data());

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        p_device.get_physical(), p_device.get_surface(), &present_mode_count,
        present_modes.data());

    VkSurfaceFormatKHR surface_format = formats.at(0);
    for (const auto &format : formats) {
        const auto color_space_right =
            format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        const auto format_right = format.format == VK_FORMAT_R8G8B8A8_SRGB;
        if (color_space_right && format_right) {
            surface_format = format;
        }
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto mode : present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = mode;
        }
    }

    VkExtent2D swap_extent = surface_capabilities.currentExtent;
    if (swap_extent.width == std::numeric_limits<uint32_t>::max()) {
        int width, height;
        glfwGetFramebufferSize(p_window, &width, &height);

        swap_extent.width =
            std::clamp(static_cast<uint32_t>(width),
                       surface_capabilities.minImageExtent.width,
                       surface_capabilities.maxImageExtent.width);

        swap_extent.height =
            std::clamp(static_cast<uint32_t>(height),
                       surface_capabilities.minImageExtent.height,
                       surface_capabilities.maxImageExtent.height);
    }

    uint32_t image_count = surface_capabilities.minImageCount + 1;
    if (image_count > surface_capabilities.maxImageCount &&
        surface_capabilities.maxImageCount != 0) {
        image_count = surface_capabilities.maxImageCount;
    }

    std::array queue_families{p_device.get_graphics_family(),
                              p_device.get_present_family()};

    const auto queue_families_same =
        p_device.get_graphics_family() == p_device.get_present_family();

    const VkSwapchainCreateInfoKHR swapchain_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = p_device.get_surface(),
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = swap_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = queue_families_same ? VK_SHARING_MODE_EXCLUSIVE
                                                : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount =
            queue_families_same ? 0
                                : static_cast<uint32_t>(queue_families.size()),
        .pQueueFamilyIndices =
            queue_families_same ? nullptr : queue_families.data(),
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    auto result = vkCreateSwapchainKHR(p_device.get(), &swapchain_info, nullptr,
                                       &swapchain);
    if (result != VK_SUCCESS) {
        fmt::println("[ERROR]: Failed to create the swapchain: {}", result);
        throw Error::VulkanError;
    }

    uint32_t image_count;
    vkGetSwapchainImagesKHR(p_device.get(), swapchain, &image_count, nullptr);

    images.resize(image_count);
    vkGetSwapchainImagesKHR(p_device.get(), swapchain, &image_count, images.data());
}

Swapchain::~Swapchain() {
    vkDestroySwapchainKHR(device.get(), swapchain, nullptr);
}
