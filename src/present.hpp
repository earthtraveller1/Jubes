#pragma once

#include "common.hpp"
#include "devices.hpp"

struct Semaphore;

class Swapchain {
  public:
    inline Swapchain(const Device &device, GLFWwindow *window) : device(device) {
        create(device, window);
    }

    void create(const Device &device, GLFWwindow *window);

    void destroy();

    struct AcquiredImage {
        uint32_t image_index;
        bool should_recreate;
    };

    // @returns two values.
    // first value is the image index
    // second value indicates whether the swapchain should be recreated or not
    AcquiredImage acquire_image(const Semaphore& signal_semaphore);

    NO_COPY(Swapchain)

    inline VkSwapchainKHR get() const { return swapchain; }

    inline VkFormat get_format() const { return image_format; }

    inline const std::vector<VkImageView> &get_image_views() const {
        return image_views;
    }

    inline const VkExtent2D &get_extent() const { return extent; }

    inline ~Swapchain() {
        destroy();
    }

  private:
    const Device &device;

    VkSwapchainKHR swapchain;
    VkFormat image_format;
    VkExtent2D extent;

    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
};
