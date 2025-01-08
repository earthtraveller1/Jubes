#pragma once

#include "devices.hpp"

class Buffer {
  public:
    enum class Type { Vertex, Index, Staging, Uniform };

    Buffer(const Device &device, VkDeviceSize size, Type type);

    NO_COPY(Buffer);

    auto copy_from(const Buffer &other,
                   VkCommandPool command_buffer) const -> void;

    auto load_using_staging(VkCommandPool command_pool, const void *data,
                            VkDeviceSize size) -> void;

    inline VkBuffer get() const { return buffer; }

    inline VkDeviceMemory get_memory() const { return memory; }

    inline VkDeviceSize get_size() const { return size; }

    inline const Device &get_device() const { return device; }

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

class StagingBuffer {
  public:
    StagingBuffer(const Device &device, VkDeviceSize size)
        : buffer(device, size, Buffer::Type::Staging) {}

    inline auto map_memory() -> void * {
        void *data;
        vkMapMemory(buffer.get_device().get(), buffer.get_memory(), 0, buffer.get_size(), 0, &data);
        return data;
    }

    inline auto unmap_memory() -> void {
        vkUnmapMemory(buffer.get_device().get(), buffer.get_memory());
    }

  private:
    Buffer buffer;
};
