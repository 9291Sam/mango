#include "render_pass.hpp"
#include "device.hpp"
#include "image.hpp"
#include "swapchain.hpp"

namespace gfx::vulkan
{

    RenderPass::RenderPass(
        std::shared_ptr<Device>    device_,
        std::shared_ptr<Swapchain> swapchain_,
        std::shared_ptr<Image2D>   depthBuffer
    )
        : device {std::move(device_)}
        , swapchain {std::move(swapchain_)}
        , depth_buffer {std::move(depthBuffer)}
        , render_pass {nullptr}
    {
        // clang-format off
        std::array<vk::AttachmentDescription, 2> attachments {
            vk::AttachmentDescription
            {
                .flags {},
                .format {this->swapchain->getSurfaceFormat().format},
                .samples {vk::SampleCountFlagBits::e1},
                .loadOp {vk::AttachmentLoadOp::eClear},
                .storeOp {vk::AttachmentStoreOp::eStore},
                .stencilLoadOp {vk::AttachmentLoadOp::eDontCare},
                .stencilStoreOp {vk::AttachmentStoreOp::eDontCare},
                .initialLayout {vk::ImageLayout::eUndefined},
                .finalLayout {vk::ImageLayout::ePresentSrcKHR},
            },
            vk::AttachmentDescription
            {
                .flags {},
                .format {this->depth_buffer->getFormat()},
                .samples {vk::SampleCountFlagBits::e1},
                .loadOp {vk::AttachmentLoadOp::eClear},
                .storeOp {vk::AttachmentStoreOp::eDontCare},
                .stencilLoadOp {vk::AttachmentLoadOp::eDontCare},
                .stencilStoreOp {vk::AttachmentStoreOp::eDontCare},
                .initialLayout {vk::ImageLayout::eUndefined},
                .finalLayout {vk::ImageLayout::eDepthStencilAttachmentOptimal},
            },
        };
        // clang-format on

        vk::AttachmentReference colorAttachmentReference {
            .attachment {0},
            .layout {vk::ImageLayout::eColorAttachmentOptimal},
        };

        vk::AttachmentReference depthAttachmentReference {
            .attachment {1},
            .layout {vk::ImageLayout::eDepthStencilAttachmentOptimal},
        };

        vk::SubpassDescription subpass {
            .flags {},
            .pipelineBindPoint {
                VULKAN_HPP_NAMESPACE::PipelineBindPoint::eGraphics},
            .inputAttachmentCount {0},
            .pInputAttachments {nullptr},
            .colorAttachmentCount {1},
            .pColorAttachments {&colorAttachmentReference},
            .pResolveAttachments {nullptr},
            .pDepthStencilAttachment {&depthAttachmentReference},
            .preserveAttachmentCount {0},
            .pPreserveAttachments {nullptr},
        };

        vk::SubpassDependency subpassDependency {
            .srcSubpass {VK_SUBPASS_EXTERNAL},
            .dstSubpass {0},
            .srcStageMask {
                vk::PipelineStageFlagBits::eColorAttachmentOutput
                | vk::PipelineStageFlagBits::eEarlyFragmentTests},
            .dstStageMask {
                vk::PipelineStageFlagBits::eColorAttachmentOutput
                | vk::PipelineStageFlagBits::eEarlyFragmentTests},
            .srcAccessMask {vk::AccessFlagBits::eNone},
            .dstAccessMask {
                vk::AccessFlagBits::eColorAttachmentWrite
                | vk::AccessFlagBits::eDepthStencilAttachmentWrite},
            .dependencyFlags {},
        };

        vk::RenderPassCreateInfo renderPassCreateInfo {
            .sType {vk::StructureType::eRenderPassCreateInfo},
            .pNext {nullptr},
            .flags {},
            .attachmentCount {attachments.size()},
            .pAttachments {attachments.data()},
            .subpassCount {1},
            .pSubpasses {&subpass},
            .dependencyCount {1},
            .pDependencies {&subpassDependency},
        };

        this->render_pass =
            this->device->asLogicalDevice().createRenderPassUnique(
                renderPassCreateInfo
            );
    }

} // namespace gfx::vulkan
