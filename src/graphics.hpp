#pragma once

#include "devices.hpp"
#include "present.hpp"

class RenderPass {
  public:
    explicit RenderPass(const Device &device, const Swapchain &swapchain);

    inline VkRenderPass get() const { return render_pass; }

    inline ~RenderPass() {
        vkDestroyRenderPass(device.get(), render_pass, nullptr);
    }

  private:
    const Device &device;

    VkRenderPass render_pass;
};
