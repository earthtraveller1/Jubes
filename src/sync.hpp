#pragma once

class Device;

#include "common.hpp"
#include "devices.hpp"

struct Semaphore {
  public:
    Semaphore(const Device &device);
    NO_COPY(Semaphore);

    inline VkSemaphore get() const { return fence; }

    inline ~Semaphore() { vkDestroySemaphore(device.get(), fence, nullptr); }

  private:
    VkSemaphore fence;
    const Device &device;
};

struct Fence {
  public:
    Fence(const Device &device, bool signaled);
    NO_COPY(Fence);

    inline VkFence get() const { return fence; }

    inline void wait() const {
        VK_ERROR(vkWaitForFences(device.get(), 1, &fence, VK_TRUE, UINT64_MAX));
    }

    inline void reset() const {
        VK_ERROR(vkResetFences(device.get(), 1, &fence));
    }

    inline ~Fence() { vkDestroyFence(device.get(), fence, nullptr); }

  private:
    VkFence fence;
    const Device &device;
};
