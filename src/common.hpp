#pragma once

enum class Error {
    Ok,
    NoValidationLayers,
    VulkanError,
    NoAdequatePhysicalDeviceError
};

template <> struct fmt::formatter<VkResult> : fmt::formatter<std::string_view> {
    format_context::iterator format(VkResult result, format_context &ctx) const;
};

template <> struct fmt::formatter<Error> : fmt::formatter<std::string_view> {
    format_context::iterator format(Error result, format_context &ctx) const;
};

#define NO_COPY(type)                                                          \
    type(type &) = delete;                                                     \
    type &operator=(type &) = delete;
