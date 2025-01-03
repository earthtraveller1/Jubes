#pragma once

#include <GLFW/glfw3.h>

#include "common.hpp"

class Device {
  public:
    Device(GLFWwindow *const window, bool enable_validation);
    Device &operator=(Device&& rhs) noexcept;

    NO_COPY(Device);

    inline VkDevice get() const { return device; }

    inline VkPhysicalDevice get_physical() const { return physical_device; }

    inline VkSurfaceKHR get_surface() const { return surface; }

    inline uint32_t get_graphics_family() const { return graphics_family; }

    inline uint32_t get_present_family() const { return present_family; }

    ~Device();

  private:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
    uint32_t graphics_family;
    uint32_t present_family;
    VkQueue graphics_queue;
    VkQueue present_queue;
};