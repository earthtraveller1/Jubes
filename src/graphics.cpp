#include "graphics.hpp"
#include "present.hpp"
#include <vulkan/vulkan_core.h>

RenderPass::RenderPass(const Device &p_device, const Swapchain &p_swapchain)
    : device(p_device) {
    VkAttachmentDescription color_attachment{
        .flags = 0,
        .format = p_swapchain.get_format(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference color_attachment_ref{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass{
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    VkRenderPassCreateInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0,
        .pDependencies = nullptr,
    };

    const auto result = vkCreateRenderPass(device.get(), &render_pass_info,
                                           nullptr, &render_pass);

    if (result != VK_SUCCESS) {
        fmt::println("[ERROR]: Failed to create a render pass: {}", result);
        throw Error::VulkanError;
    }
}

Framebuffers::Framebuffers(const Device &p_device, const Swapchain &p_swapchain,
                           const RenderPass &p_render_pass)
    : device(p_device) {
    framebuffers.reserve(p_swapchain.get_image_views().size());

    for (const auto image_view : p_swapchain.get_image_views()) {
        const VkFramebufferCreateInfo fb_info{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = p_render_pass.get(),
            .attachmentCount = 1,
            .pAttachments = &image_view,
            .width = p_swapchain.get_extent().width,
            .height = p_swapchain.get_extent().height,
            .layers = 1,
        };

        VkFramebuffer fb;
        const auto result =
            vkCreateFramebuffer(p_device.get(), &fb_info, nullptr, &fb);
        if (result != VK_SUCCESS) {
            fmt::println("[ERROR]: Failed to create a framebuffer: {}", result);
            throw Error::VulkanError;
        }

        framebuffers.push_back(fb);
    }
}
