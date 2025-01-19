#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <vulkan/vulkan_core.h>

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

    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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

    std::array vertices = {
        Vertex{{0.5, -0.5, 0.0}},
        Vertex{{0.5, 0.5, 0.0}},
        Vertex{{-0.5, 0.5, 0.0}},
        Vertex{{-0.5, -0.5, 0.0}},
    };

    std::array<uint16_t, 6> indices{
        0, 1, 2, 0, 2, 3,
    };

    Buffer vertex_buffer{device, vertices.size() * sizeof(vertices[0]),
                         Buffer::Type::Vertex};
    vertex_buffer.load_using_staging(command_pool, vertices.data(),
                                     vertices.size() * sizeof(vertices[0]));

    Buffer index_buffer{device, indices.size() * sizeof(vertices[0]),
                        Buffer::Type::Index};
    index_buffer.load_using_staging(command_pool, indices.data(),
                                    indices.size() * sizeof(indices[0]));

    const auto command_buffer = command_pool.allocate_buffer();

    while (!glfwWindowShouldClose(window)) {
        const VkFence frame_fence_raw = frame_fence.get();
        vkWaitForFences(device.get(), 1, &frame_fence_raw, VK_TRUE, UINT64_MAX);
        vkResetFences(device.get(), 1, &frame_fence_raw);

        auto [image_index, should_recreate] =
            swapchain.acquire_image(image_acquired_semaphore);
        if (should_recreate) {
            vkDeviceWaitIdle(device.get());
            framebuffers.destroy();
            swapchain.destroy();
            swapchain.create(device, window);
            framebuffers.create(device, swapchain, render_pass);

            continue;
        }

        vkResetCommandBuffer(command_buffer, 0);

        const VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };

        VK_ERROR(vkBeginCommandBuffer(command_buffer, &begin_info));

        render_pass.begin(command_buffer, swapchain,
                          framebuffers.get(image_index), {1.0, 0.5, 0.5, 1.0});

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline.get());

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const VkViewport viewport{
            .x = 0,
            .y = 0,
            .width = static_cast<float>(width),
            .height = static_cast<float>(height),
            .minDepth = 0,
            .maxDepth = 1,
        };
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        const VkRect2D scissor{.offset =
                                   {
                                       .x = 0,
                                       .y = 0,
                                   },
                               .extent = {
                                   .width = static_cast<uint32_t>(width),
                                   .height = static_cast<uint32_t>(height),
                               }};
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        const std::array buffers{vertex_buffer.get()};
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers.data(), &offset);
        vkCmdBindIndexBuffer(command_buffer, index_buffer.get(), offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(command_buffer, 6, 1, 0, 0, 0);

        vkCmdEndRenderPass(command_buffer);

        vkEndCommandBuffer(command_buffer);

        device.submit_to_graphics(command_buffer, image_acquired_semaphore,
                                  rendering_done_semaphore, frame_fence);
        should_recreate =
            device.present(swapchain, rendering_done_semaphore, image_index);
        if (should_recreate) {
            vkDeviceWaitIdle(device.get());
            framebuffers.destroy();
            swapchain.destroy();
            swapchain.create(device, window);
            framebuffers.create(device, swapchain, render_pass);
        }

        glfwPollEvents();
    }

    vkDeviceWaitIdle(device.get());

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
} catch (Error error) {
    fmt::println("[ERROR]: {}", error);
}
