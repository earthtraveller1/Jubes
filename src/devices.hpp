#pragma once

#include <GLFW/glfw3.h>

#include "common.hpp"

class Swapchain;
struct Semaphore;
struct Fence;

class Device {
  public:
    Device(GLFWwindow *const window, bool enable_validation);
    Device &operator=(Device &&rhs) noexcept;

    NO_COPY(Device);

    inline VkDevice get() const { return device; }

    inline VkPhysicalDevice get_physical() const { return physical_device; }

    inline VkSurfaceKHR get_surface() const { return surface; }

    inline uint32_t get_graphics_family() const { return graphics_family; }

    inline uint32_t get_present_family() const { return present_family; }

    inline VkQueue get_graphics_queue() const { return graphics_queue; }

    inline VkQueue get_present_queue() const { return present_queue; }

    void submit_to_graphics(VkCommandBuffer command_buffer,
                            const Semaphore &wait_semaphore,
                            const Semaphore &signal_semaphore,
                            const Fence &fence) const;

    // returns - whether you should recreate the swapchain or not.
    bool present(const Swapchain &swapchain, const Semaphore &wait_semaphore,
                 uint32_t image_index) const;

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

struct CommandPool {
    VkCommandPool pool;
    const Device &device;

    CommandPool(const Device &p_device);

    NO_COPY(CommandPool);

    auto allocate_buffer() const -> VkCommandBuffer;

    inline ~CommandPool() { vkDestroyCommandPool(device.get(), pool, nullptr); }
};
