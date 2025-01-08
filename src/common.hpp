#pragma once

enum class Error {
    Ok,
    NoValidationLayers,
    VulkanError,
    NoAdequatePhysicalDeviceError,
    FileOpenError,
};

template <> struct fmt::formatter<VkResult> : fmt::formatter<std::string_view> {
    format_context::iterator format(VkResult result, format_context &ctx) const;
};

template <> struct fmt::formatter<Error> : fmt::formatter<std::string_view> {
    format_context::iterator format(Error result, format_context &ctx) const;
};

std::vector<char> read_as_bytes(std::string_view file_name);

#define NO_COPY(type)                                                          \
    type(type &) = delete;                                                     \
    type &operator=(type &) = delete;

#define VK_ERROR(exp)                                                          \
    do {                                                                       \
        const auto result = exp;                                               \
        if (result != VK_SUCCESS) {                                            \
            fmt::println("[ERROR]: Failed to " #exp ": {}", result);           \
            throw Error::VulkanError;                                          \
        }                                                                      \
    } while (0)
