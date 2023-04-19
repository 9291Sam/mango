#include "frame.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "render_pass.hpp"
#include "swapchain.hpp"

namespace gfx::vulkan
{
    Frame::Frame(
        std::shared_ptr<Device>     device_,
        std::shared_ptr<Swapchain>  swapchain_,
        std::shared_ptr<RenderPass> renderPass)
        : device {std::move(device_)}
        , swapchain {std::move(swapchain_)}
        , render_pass {std::move(renderPass)}
        , image_available {nullptr}
        , render_finished {nullptr}
        , frame_in_flight {nullptr}
    {
        const vk::SemaphoreCreateInfo semaphoreCreateInfo {
            .sType {vk::StructureType::eSemaphoreCreateInfo},
            .pNext {nullptr},
            .flags {},
        };

        const vk::FenceCreateInfo fenceCreateInfo {
            .sType {vk::StructureType::eFenceCreateInfo},
            .pNext {nullptr},
            .flags {vk::FenceCreateFlagBits::eSignaled},
        };

        this->image_available =
            this->device->asLogicalDevice().createSemaphoreUnique(
                semaphoreCreateInfo);
        this->render_finished =
            this->device->asLogicalDevice().createSemaphoreUnique(
                semaphoreCreateInfo);
        this->frame_in_flight =
            this->device->asLogicalDevice().createFenceUnique(fenceCreateInfo);
    }

    bool Frame::render(
        const std::vector<vk::UniqueFramebuffer>& framebuffers,
        const FlatPipeline&                       pipeline,
        const StagedBuffer&                       vertexBuffer)
    {
        std::optional<bool> returnValue = std::nullopt;

        this->device->accessGraphicsBuffer(
            [&](vk::Queue queue, vk::CommandBuffer commandBuffer)
            {
                const std::uint64_t timeout =
                    std::numeric_limits<std::uint64_t>::max();

                {
                    vk::Result result =
                        this->device->asLogicalDevice().waitForFences(
                            *this->frame_in_flight, true, timeout);

                    util::assertFatal(
                        result == vk::Result::eSuccess,
                        "Failed to wait for render fence {}",
                        vk::to_string(result));
                }

                std::uint32_t nextImageIndex;
                {
                    const auto [result, maybeNextFrameBufferIndex] =
                        this->device->asLogicalDevice().acquireNextImageKHR(
                            **this->swapchain, timeout, *this->image_available);

                    if (result == vk::Result::eErrorOutOfDateKHR
                        || result == vk::Result::eSuboptimalKHR)
                    {
                        returnValue = true;
                        return;
                    }
                    else
                    {
                        util::assertFatal(
                            result == vk::Result::eSuccess,
                            "Failed to acquire next image {}",
                            vk::to_string(result));
                    }

                    nextImageIndex =
                        static_cast<std::uint32_t>(maybeNextFrameBufferIndex);
                }

                this->device->asLogicalDevice().resetFences(
                    *this->frame_in_flight);

                const vk::CommandBufferBeginInfo commandBufferBeginInfo {
                    .sType {vk::StructureType::eCommandBufferBeginInfo},
                    .pNext {nullptr},
                    .flags {},
                    .pInheritanceInfo {nullptr},
                };

                commandBuffer.begin(commandBufferBeginInfo);

                // union initialization syntax my beloved :heart:
                // clang-format off
                std::array<vk::ClearValue, 2> clearValues
                {
                    vk::ClearValue
                    {
                        .color
                        {
                            vk::ClearColorValue {
                                std::array<float, 4> {0.02f, 0.02f, 0.02f, 1.0f}
                            }
                        }
                    },
                    vk::ClearValue
                    {
                        .depthStencil
                        {
                            vk::ClearDepthStencilValue
                            {
                                .depth {1.0f},
                                .stencil {0}
                            }
                        }
                    }
                };
                // clang-format on

                vk::RenderPassBeginInfo renderPassBeginInfo {
                    .sType {vk::StructureType::eRenderPassBeginInfo},
                    .pNext {nullptr},
                    .renderPass {**this->render_pass},
                    .framebuffer {*framebuffers.at(nextImageIndex)},
                    .renderArea {vk::Rect2D {
                        .offset {0, 0},
                        .extent {this->swapchain->getExtent()},
                    }},
                    .clearValueCount {clearValues.size()},
                    .pClearValues {clearValues.data()},
                };

                commandBuffer.beginRenderPass(
                    renderPassBeginInfo, vk::SubpassContents::eInline);

                // loops lol
                commandBuffer.bindPipeline(
                    vk::PipelineBindPoint::eGraphics, *pipeline);

                commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});

                commandBuffer.draw(3, 1, 0, 0);

                commandBuffer.endRenderPass();
                commandBuffer.end();

                const vk::PipelineStageFlags waitStages =
                    vk::PipelineStageFlagBits::eColorAttachmentOutput;

                vk::SubmitInfo submitInfo {
                    .sType {vk::StructureType::eSubmitInfo},
                    .pNext {nullptr},
                    .waitSemaphoreCount {1},
                    .pWaitSemaphores {&*this->image_available},
                    .pWaitDstStageMask {&waitStages},
                    .commandBufferCount {1},
                    .pCommandBuffers {&commandBuffer},
                    .signalSemaphoreCount {1},
                    .pSignalSemaphores {&*this->render_finished},
                };

                queue.submit(submitInfo, *this->frame_in_flight);

                vk::SwapchainKHR swapchainPtr = **this->swapchain;

                vk::PresentInfoKHR presentInfo {
                    .sType {vk::StructureType::ePresentInfoKHR},
                    .pNext {nullptr},
                    .waitSemaphoreCount {1},
                    .pWaitSemaphores {&*this->render_finished},
                    .swapchainCount {1},
                    .pSwapchains {&swapchainPtr},
                    .pImageIndices {&nextImageIndex},
                    .pResults {nullptr},
                };

                try
                {
                    std::ignore = queue.presentKHR(presentInfo);
                }
                catch (const vk::OutOfDateKHRError&)
                {
                    returnValue = true;
                    return;
                }

                const vk::Result result =
                    this->device->asLogicalDevice().waitForFences(
                        *this->frame_in_flight, true, timeout);

                util::assertFatal(
                    result == vk::Result::eSuccess,
                    "Failed to wait for frame to complete drawing {}",
                    vk::to_string(result));

                returnValue = false;
                return;
            });

        return returnValue.value();
    }
} // namespace gfx::vulkan
