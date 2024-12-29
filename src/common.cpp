#include "common.hpp"

fmt::format_context::iterator
fmt::formatter<VkResult>::format(VkResult p_result,
                                 format_context &p_ctx) const {
    std::string_view result;

    switch (p_result) {
    case VK_SUCCESS:
        result = "VK_SUCCESS";
        break;
    case VK_NOT_READY:
        result = "VK_NOT_READY";
        break;
    case VK_TIMEOUT:
        result = "VK_TIMEOUT";
        break;
    case VK_EVENT_SET:
        result = "VK_EVENT_SET";
        break;
    case VK_EVENT_RESET:
        result = "VK_EVENT_RESET";
        break;
    case VK_INCOMPLETE:
        result = "VK_INCOMPLETE";
        break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        result = "VK_ERROR_OUT_OF_HOST_MEMORY";
        break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        result = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        break;
    case VK_ERROR_INITIALIZATION_FAILED:
        result = "VK_ERROR_INITIALIZATION_FAILED";
        break;
    case VK_ERROR_DEVICE_LOST:
        result = "VK_ERROR_DEVICE_LOST";
        break;
    case VK_ERROR_MEMORY_MAP_FAILED:
        result = "VK_ERROR_MEMORY_MAP_FAILED";
        break;
    case VK_ERROR_LAYER_NOT_PRESENT:
        result = "VK_ERROR_LAYER_NOT_PRESENT";
        break;
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        result = "VK_ERROR_EXTENSION_NOT_PRESENT";
        break;
    case VK_ERROR_FEATURE_NOT_PRESENT:
        result = "VK_ERROR_FEATURE_NOT_PRESENT";
        break;
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        result = "VK_ERROR_INCOMPATIBLE_DRIVER";
        break;
    case VK_ERROR_TOO_MANY_OBJECTS:
        result = "VK_ERROR_TOO_MANY_OBJECTS";
        break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        result = "VK_ERROR_FORMAT_NOT_SUPPORTED";
        break;
    case VK_ERROR_SURFACE_LOST_KHR:
        result = "VK_ERROR_SURFACE_LOST_KHR";
        break;
    case VK_SUBOPTIMAL_KHR:
        result = "VK_SUBOPTIMAL_KHR";
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
        result = "VK_ERROR_OUT_OF_DATE_KHR";
        break;
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        result = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        break;
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        result = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        break;
    case VK_ERROR_VALIDATION_FAILED_EXT:
        result = "VK_ERROR_VALIDATION_FAILED_EXT";
        break;
    case VK_RESULT_MAX_ENUM:
        result = "VK_RESULT_MAX_ENUM";
        break;
    }

    return fmt::formatter<std::string_view>::format(result, p_ctx);
}

fmt::format_context::iterator
fmt::formatter<Error>::format(Error p_result, format_context &p_ctx) const {
    std::string_view result;

    switch (p_result) {
    case Error::Ok:
        result = "Error::Ok";
        break;
    case Error::VulkanError:
        result = "Error::VulkanError";
        break;
    case Error::NoValidationLayers:
        result = "Error::NoValidationLayers";
        break;
    case Error::NoAdequatePhysicalDeviceError:
        result = "Error::NoAdequatePhysicalDeviceError";
        break;
    case Error::FileOpenError:
        result = "Error::FileOpenError";
        break;
    }

    return fmt::formatter<std::string_view>::format(result, p_ctx);
}

auto read_as_bytes(std::string_view file_name) -> std::vector<char> {
    std::ifstream file(file_name.data(), std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw Error::FileOpenError;
    }

    const auto file_size = file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);

    return buffer;
}
