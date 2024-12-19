#pragma once

#include <GLFW/glfw3.h>

#include "common.hpp"

class Device {
  public:
    Device(GLFWwindow *const window, bool enable_validation);
    Device &operator=(Device&& rhs) noexcept;

    NO_COPY(Device)

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