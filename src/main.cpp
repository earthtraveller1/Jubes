#include <GLFW/glfw3.h>
#include <fmt/format.h>

#include "buffers.hpp"
#include "devices.hpp"
#include "graphics.hpp"
#include "present.hpp"

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

    std::array<Vertex, 3> vertices = {
        Vertex{{0.0, 0.5, 0.0}},
        Vertex{{0.5, 0.5, 0.0}},
        Vertex{{-0.5, 0.5, 0.0}},
    };

    Buffer buffer{device, vertices.size() * sizeof(vertices[0]),
                  Buffer::Type::Vertex};
    buffer.load_using_staging(command_pool, vertices.data(),
                              vertices.size() * sizeof(vertices[0]));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
} catch (Error error) {
    fmt::println("[ERROR]: {}", error);
}
