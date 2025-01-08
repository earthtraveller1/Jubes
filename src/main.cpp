#include <GLFW/glfw3.h>
#include <fmt/format.h>

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
    Swapchain swapchain{device, window};

    RenderPass render_pass{device, swapchain};
    Framebuffers framebuffers{device, swapchain, render_pass};
    GraphicsPipeline pipeline{
        device, render_pass, "shaders/main.vert.spv", "shaders/main.frag.spv",
        {},     {},
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
} catch (Error error) {
    fmt::println("[ERROR]: {}", error);
}
