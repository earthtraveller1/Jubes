#pragma once

#include <GLFW/glfw3.h>

#include "common.hpp"

struct Device {
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice device;
	uint32_t graphics_family;
	uint32_t present_family;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkSurfaceKHR surface;

	static std::tuple<Device, Error> create(GLFWwindow* const window, bool enable_validation);
};