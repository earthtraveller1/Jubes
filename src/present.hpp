#pragma once

#include "common.hpp"
#include "devices.hpp"

class Swapchain {
  public:
    Swapchain(const Device& device, GLFWwindow* window);

    NO_COPY(Swapchain)

    inline VkFormat get_format() const { return image_format; }

    ~Swapchain();

    private:
        const Device& device;

        VkSwapchainKHR swapchain;
        VkFormat image_format;
        VkExtent2D extent;

        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
};
