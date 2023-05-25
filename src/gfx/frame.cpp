#include "frame.hpp"
#include "camera.hpp"
#include "object.hpp"
#include "transform.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/device.hpp"
#include "vulkan/pipelines.hpp"
#include "vulkan/render_pass.hpp"
#include "vulkan/swapchain.hpp"

namespace gfx
{
    Frame::Frame(
        std::shared_ptr<vulkan::Device>                     device_,
        std::shared_ptr<vulkan::Swapchain>                  swapchain_,
        std::shared_ptr<vulkan::RenderPass>                 renderPass,
        std::shared_ptr<std::vector<vk::UniqueFramebuffer>> framebuffers_)
        : device {std::move(device_)}
        , swapchain {std::move(swapchain_)}
        , render_pass {std::move(renderPass)}
        , framebuffers {std::move(framebuffers_)}
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
        const Camera&                                           camera,
        vk::Extent2D                                            size,
        const std::map<vulkan::PipelineType, vulkan::Pipeline>& pipelineMap,
        std::span<const Object*>                                unsortedObjects)
    {
        std::optional<bool> returnValue = std::nullopt;

        std::vector<const Object*> sortedObjects;
        sortedObjects.insert(
            sortedObjects.cend(),
            unsortedObjects.begin(),
            unsortedObjects.end());
        std::ranges::sort(
            sortedObjects,
            [](const Object* l, const Object* r)
            {
                return *l < *r;
            });

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
                                std::array<float, 4> {0.01f, 0.3f, 0.4f, 1.0f}
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
                    .framebuffer {*this->framebuffers->at(nextImageIndex)},
                    .renderArea {vk::Rect2D {
                        .offset {0, 0},
                        .extent {this->swapchain->getExtent()},
                    }},
                    .clearValueCount {clearValues.size()},
                    .pClearValues {clearValues.data()},
                };

                commandBuffer.beginRenderPass(
                    renderPassBeginInfo, vk::SubpassContents::eInline);

                // std::size_t currentBoundPipelineIndex = ~0UZ;
                BindState bindState {};

                for (const Object* o : sortedObjects)
                {
                    o->bind(commandBuffer, bindState, pipelineMap);
                    o->setPushConstants(
                        commandBuffer,
                        pipelineMap.at(bindState.current_pipeline),
                        camera,
                        size);
                    o->draw(commandBuffer);
                }

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

                {
                    VkResult result =
                        VULKAN_HPP_DEFAULT_DISPATCHER.vkQueuePresentKHR(
                            static_cast<VkQueue>(queue),
                            reinterpret_cast<const VkPresentInfoKHR*>(
                                &presentInfo));

                    if (result == VK_ERROR_OUT_OF_DATE_KHR
                        || result == VK_SUBOPTIMAL_KHR)
                    {
                        returnValue = true;
                        return;
                    }
                    else if (result == VK_SUCCESS)
                    {}
                    else
                    {
                        util::panic(
                            "Unhandled error! {}",
                            vk::to_string(vk::Result {result}));
                    }
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
} // namespace gfx
