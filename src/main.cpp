#include <GLFW/glfw3.h>
#include <fmt/format.h>

#include "buffers.hpp"
#include "devices.hpp"
#include "graphics.hpp"
#include "present.hpp"
#include "sync.hpp"

constexpr auto WINDOW_WIDTH = 1280;
constexpr auto WINDOW_HEIGHT = 720;

int main() try {
    if (!glfwInit()) {
        fmt::println("Failed to initialize GLFW.");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback([](int error_code, const char *description) {
        fmt::println("[GLFW Error {}]: {}", error_code, description);
    });

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Jubes",
                                         nullptr, nullptr);

    if (window == nullptr) {
        fmt::println("Failed to create the GLFW window.");
        return EXIT_FAILURE;
    }

    Device device{window, true};
    CommandPool command_pool{device};
    Swapchain swapchain{device, window};

    RenderPass render_pass{device, swapchain};
    Framebuffers framebuffers{device, swapchain, render_pass};
    GraphicsPipeline pipeline{
        device, render_pass, "shaders/main.vert.spv", "shaders/main.frag.spv",
        {},     {},
    };
    
    Fence frame_fence{device, true};
    Semaphore image_acquired_semaphore{device};
    Semaphore rendering_done_semaphore{device};

    std::array<Vertex, 3> vertices = {
        Vertex{{0.0, 0.5, 0.0}},
        Vertex{{0.5, 0.5, 0.0}},
        Vertex{{-0.5, 0.5, 0.0}},
    };

    Buffer buffer{device, vertices.size() * sizeof(vertices[0]),
                  Buffer::Type::Vertex};
    buffer.load_using_staging(command_pool, vertices.data(),
                              vertices.size() * sizeof(vertices[0]));
                              
    const auto command_buffer = command_pool.allocate_buffer();

    while (!glfwWindowShouldClose(window)) {
        const VkFence frame_fence_raw = frame_fence.get();
        vkWaitForFences(device.get(), 1, &frame_fence_raw, VK_TRUE, UINT64_MAX);
        vkResetFences(device.get(), 1, &frame_fence_raw);
        
        uint32_t image_index;
        VK_ERROR(vkAcquireNextImageKHR(device.get(), swapchain.get(), UINT64_MAX, image_acquired_semaphore.get(), VK_NULL_HANDLE, &image_index));

        vkResetCommandBuffer(command_buffer, 0);
        
        const VkCommandBufferBeginInfo begin_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };
        
        VK_ERROR(vkBeginCommandBuffer(command_buffer, &begin_info));
        
        const VkClearValue clear_value {
            .color = {
                .float32 = { 1.0, 0.5, 0.5, 1.0 }
            }
        };

        const VkRenderPassBeginInfo render_pass_begin_info {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = render_pass.get(),
            .framebuffer = framebuffers.get(image_index),
            .renderArea = VkRect2D {
                .offset = VkOffset2D {
                    .x = 0,
                    .y = 0,
                },
                .extent = swapchain.get_extent(),
            },
            .clearValueCount = 1,
            .pClearValues = &clear_value,
        };
        
        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdEndRenderPass(command_buffer);
        
        vkEndCommandBuffer(command_buffer);
        
        const auto image_acquired_semaphore_raw = image_acquired_semaphore.get();
        const auto render_done_semaphore_raw = rendering_done_semaphore.get();
        
        const VkPipelineStageFlags wait_stage =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        const VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &image_acquired_semaphore_raw,
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &render_done_semaphore_raw,
        };

        VK_ERROR(vkQueueSubmit(
            device.get_graphics_queue(), 1, &submit_info, frame_fence.get()
        ));

        VkSwapchainKHR swapchain_raw = swapchain.get();

        const VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &render_done_semaphore_raw,
            .swapchainCount = 1,
            .pSwapchains = &swapchain_raw,
            .pImageIndices = &image_index,
            .pResults = nullptr,
        };
        
        VK_ERROR(vkQueuePresentKHR(device.get_present_queue(), &present_info));
        
        glfwPollEvents();
    }
    
    vkDeviceWaitIdle(device.get());

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
} catch (Error error) {
    fmt::println("[ERROR]: {}", error);
}
