#include "buffers.hpp"

Buffer::Buffer(
    const Device &device, VkDeviceSize size, type_t type
): device(device) {
    const VkBufferCreateInfo buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage =
            [&]() {
                switch (type) {
                case type_t::vertex:
                    return static_cast<VkBufferUsageFlags>(
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT
                    );
                case type_t::index:
                    return static_cast<VkBufferUsageFlags>(
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT
                    );
                case type_t::staging:
                    return static_cast<VkBufferUsageFlags>(
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                    );
                case type_t::uniform:
                    return static_cast<VkBufferUsageFlags>(
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                    );
                }
            }(),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    VkBuffer buffer;
    auto result =
        vkCreateBuffer(device.get(), &buffer_create_info, nullptr, &buffer);

    if (result != VK_SUCCESS) {
        fmt::println("[ERROR]: Failed to create a buffer: {}", result);
        throw Error::VulkanError;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(
        device.get(), buffer, &memory_requirements
    );

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device.get_physical(), &memory_properties);

    VkMemoryPropertyFlags memory_property_flags = [&]() {
        switch (type) {
        case type_t::vertex:
            return static_cast<VkMemoryPropertyFlags>(
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );
        case type_t::index:
            return static_cast<VkMemoryPropertyFlags>(
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );
        case type_t::staging:
            return static_cast<VkMemoryPropertyFlags>(
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
        case type_t::uniform:
            return static_cast<VkMemoryPropertyFlags>(
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
        }
    }();

    std::optional<uint32_t> memory_type_index;

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        const auto property_flags =
            memory_properties.memoryTypes[i].propertyFlags;

        const auto has_type_bit =
            (memory_requirements.memoryTypeBits & (1 << i)) != 0;

        const auto has_property_flags =
            (property_flags & memory_property_flags) == memory_property_flags;

        if (has_type_bit && has_property_flags) {
            memory_type_index = i;
        }
    }

    if (!memory_type_index.has_value()) {
        throw std::runtime_error("Could not find suitable memory type.");
    }

    const VkMemoryAllocateInfo memory_allocate_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index.value(),
    };

    VkDeviceMemory memory;
    VK_ERROR(vkAllocateMemory(
        device.get(), &memory_allocate_info, nullptr, &memory
    ));

    vkBindBufferMemory(device.get(), buffer, memory, 0);
}

auto Buffer::copy_from(const Buffer &other, VkCommandPool command_pool)
    const -> void {
    // Pick the smallest of the two sizes
    const auto size = std::min(other.size, this->size);

    const VkCommandBufferAllocateInfo command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer p_command_buffer;
    VK_ERROR(vkAllocateCommandBuffers(
        device.get(), &command_buffer_allocate_info, &p_command_buffer
    ));

    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    VK_ERROR(vkBeginCommandBuffer(p_command_buffer, &begin_info));

    const VkBufferCopy copy_region{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };

    vkCmdCopyBuffer(
        p_command_buffer, other.buffer, this->buffer, 1, &copy_region
    );

    VK_ERROR(vkEndCommandBuffer(p_command_buffer));

    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &p_command_buffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };

    VK_ERROR(
        vkQueueSubmit(device.get_graphics_queue(), 1, &submit_info, VK_NULL_HANDLE)
    );
}
