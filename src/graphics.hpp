#pragma once

#include "devices.hpp"
#include "present.hpp"

class RenderPass {
  public:
    explicit RenderPass(const Device &device, const Swapchain &swapchain);

    NO_COPY(RenderPass);

    inline VkRenderPass get() const { return render_pass; }

    void begin(VkCommandBuffer command_buffer, const Swapchain& swapchain, VkFramebuffer framebuffer, glm::vec4 clear_color) const;

    inline ~RenderPass() {
        vkDestroyRenderPass(device.get(), render_pass, nullptr);
    }

  private:
    const Device &device;

    VkRenderPass render_pass;
};

struct Framebuffers {
  public:
    Framebuffers(const Device &device, const Swapchain &swapchain,
                 const RenderPass &render_pass): device(device) {
        create(device, swapchain, render_pass);
    }

    void create(const Device &device, const Swapchain &swapchain, const RenderPass &render_pass);

    inline VkFramebuffer get(size_t i) const { return framebuffers.at(i); }

    NO_COPY(Framebuffers);

    inline void destroy() {
        for (const auto fb : framebuffers) {
            vkDestroyFramebuffer(device.get(), fb, nullptr);
        }

        framebuffers.clear();
    }

    inline ~Framebuffers() {
        destroy();
    }

  private:
    const Device &device;

    std::vector<VkFramebuffer> framebuffers;
};

class GraphicsPipeline {
  public:
    GraphicsPipeline(
        const Device &device, const RenderPass &p_render_pass,
        std::string_view vertex_shader_path,
        std::string_view fragment_shader_path,
        std::span<const VkPushConstantRange> push_constant_ranges,
        std::span<const VkDescriptorSetLayout> descriptor_set_layouts);

    NO_COPY(GraphicsPipeline);

    inline ~GraphicsPipeline() {
        vkDestroyPipelineLayout(device.get(), layout, nullptr);
        vkDestroyPipeline(device.get(), pipeline, nullptr);
    }

  private:
    VkPipeline pipeline;
    VkPipelineLayout layout;

    const RenderPass &render_pass;

    const Device &device;
};

struct Vertex {
    glm::vec3 position;
};

constexpr VkVertexInputBindingDescription vertex_input_binding_description{
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};

constexpr static std::array vertex_attribute_descriptions{
    VkVertexInputAttributeDescription{
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, position),
    },
};
