#include <iostream>

#include <GLFW/glfw3.h>
#include <fmt/format.h>

constexpr auto WINDOW_WIDTH = 1280;
constexpr auto WINDOW_HEIGHT = 720;

int main() {
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

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}