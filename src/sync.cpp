#include "sync.hpp"

Semaphore::Semaphore(const Device& device): device(device) {
    VkSemaphoreCreateInfo semaphore_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    
    VK_ERROR(vkCreateSemaphore(device.get(), &semaphore_info, nullptr, &fence));
}

Fence::Fence(const Device& device): device(device) {
    VkFenceCreateInfo fence_info {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    
    VK_ERROR(vkCreateFence(device.get(), &fence_info, nullptr, &fence));
}