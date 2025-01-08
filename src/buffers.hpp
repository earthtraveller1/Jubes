#pragma once

#include "devices.hpp"

class Buffer {
    enum class type_t { vertex, index, staging, uniform };


    Buffer(const Device &device, VkDeviceSize size, type_t type);

    NO_COPY(Buffer);

    auto copy_from(const Buffer &other,
                   VkCommandPool command_buffer) const -> void;

    auto load_using_staging(VkCommandPool command_pool, const void *data,
                            VkDeviceSize size) -> void;

    ~Buffer() {
        vkDestroyBuffer(device.get(), buffer, nullptr);
        vkFreeMemory(device.get(), memory, nullptr);
    }

  private:
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;

    const Device &device;
};
