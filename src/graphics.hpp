#pragma once

#include "devices.hpp"
#include "present.hpp"

class RenderPass {
  public:
    explicit RenderPass(const Device &device, const Swapchain &swapchain);

    NO_COPY(RenderPass);

    inline VkRenderPass get() const { return render_pass; }

    inline ~RenderPass() {
        vkDestroyRenderPass(device.get(), render_pass, nullptr);
    }

  private:
    const Device &device;

    VkRenderPass render_pass;
};

struct Framebuffers {
public:
    Framebuffers(const Device& device, const Swapchain& swapchain, const RenderPass& render_pass);

    inline VkFramebuffer get(size_t i) const { return framebuffers.at(i); }

    NO_COPY(Framebuffers);

    inline ~Framebuffers() {
        for (const auto fb : framebuffers) {
            vkDestroyFramebuffer(device.get(), fb, nullptr);
        }
    }

private:
    const Device &device;

    std::vector<VkFramebuffer> framebuffers;
};
