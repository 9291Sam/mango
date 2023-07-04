#include "renderer.hpp"
#include "camera.hpp"
#include "frame.hpp"
#include "gfx/camera.hpp"
#include "object.hpp"
#include "transform.hpp"
#include "vulkan/allocator.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/device.hpp"
#include "vulkan/gpu_data.hpp"
#include "vulkan/image.hpp"
#include "vulkan/includes.hpp"
#include "vulkan/instance.hpp"
#include "vulkan/pipelines.hpp"
#include "vulkan/render_pass.hpp"
#include "vulkan/swapchain.hpp"
#include <iostream>
#include <util/log.hpp>
#include <util/threads.hpp>

gfx::Renderer::Renderer()
    : window {{1'920, 1'080}, "Mango"}
    , instance {nullptr}
    , device {nullptr}
    , allocator {nullptr}
    , swapchain {nullptr}
    , depth_buffer {nullptr}
    , render_pass {nullptr}
    , pipeline_map {}
    , render_index {0}
    , frames {nullptr}
{
    const vk::DynamicLoader         dl;
    const PFN_vkGetInstanceProcAddr dynVkGetInstanceProcAddr =
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(dynVkGetInstanceProcAddr);

    this->instance = std::make_unique<vulkan::Instance>(
        dynVkGetInstanceProcAddr,
        [&](vk::Instance instance_)
        {
            VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);
        });

    this->draw_surface = std::make_shared<vk::UniqueSurfaceKHR>(
        this->window.createSurface(**this->instance));

    this->device = std::make_shared<vulkan::Device>(
        this->instance,
        **this->draw_surface,
        [&](vk::Device device_)
        {
            VULKAN_HPP_DEFAULT_DISPATCHER.init(**this->instance, device_);
        });

    this->allocator = std::make_shared<vulkan::Allocator>(
        this->instance,
        this->device,
        dynVkGetInstanceProcAddr,
        dl.getProcAddress<PFN_vkGetDeviceProcAddr>("vkGetDeviceProcAddr"));

    this->initializeRenderer();

    util::logLog("Renderer initialization complete");
}

gfx::Renderer::~Renderer()
{
    this->device->asLogicalDevice().waitIdle();
}

// VoxelObject
// gfx::Renderer::createVoxelObject(std::span<glm::vec3> voxelPositions) const
// {
//     return VoxelObject {this->device, this->allocator, voxelPositions};
// }

// Object gfx::Renderer::createObject(
//     PipelineType                    pipelineType,
//     std::span<const vulkan::Vertex> vertices,
//     std::span<const vulkan::Index>  indices) const
// {
//     util::assertFatal(
//         vertices.size() > 0, "Span of size 0 was passed for vertices");
//     util::assertFatal(
//         indices.size() > 0, "Span of size 0 was passed for indices");

//     return Object {
//         this->allocator,
//         static_cast<std::size_t>(pipelineType),
//         vertices,
//         indices,
//     };
// }
bool gfx::Renderer::shouldClose() const
{
    return this->window.shouldClose();
}

float gfx::Renderer::getDeltaTimeSeconds() const
{
    return this->window.getDeltaTimeSeconds();
}

float gfx::Renderer::getActionAmount(Window::Action action) const
{
    return (this->window.isActionActive(action) ? 1.0f : 0.0f)
         * this->window.getDeltaTimeSeconds();
}

gfx::Window::Delta gfx::Renderer::getMouseDeltaRadians() const
{
    // each value from -1.0 -> 1.0 representing how much it moved on the screen
    const auto [nDeltaX, nDeltaY] = this->window.getMouseDelta();

    const auto deltaRadiansX = (nDeltaX / 2) * this->getFovXRadians();
    const auto deltaRadiansY = (nDeltaY / 2) * this->getFovYRadians();

    return Window::Delta {.x {deltaRadiansX}, .y {deltaRadiansY}};
}

float gfx::Renderer::getFovYRadians() const // NOLINT: may change in the future
{
    return glm::radians(70.f);
}

float gfx::Renderer::getFovXRadians() const
{
    // assume that the window is never stretched with an image.
    // i.e 100x pixels are 100y pixels if rotated
    const auto [width, height] = this->window.size();

    return this->getFovYRadians() * static_cast<float>(width)
         / static_cast<float>(height);
}

void gfx::Renderer::drawObjects(
    const Camera& camera, std::span<const Object*> objects)
{
    this->render_index = (this->render_index + 1) % Renderer::MaxFramesInFlight;

    if (this->frames.at(this->render_index)
            ->render(
                camera,
                this->swapchain->getExtent(),
                this->pipeline_map,
                objects))
    {
        this->resize();
    }

    this->window.pollEvents();
}

void gfx::Renderer::resize()
{
    this->window.blockThisThreadWhileMinimized();
    this->device->asLogicalDevice().waitIdle();

    for (std::unique_ptr<Frame>& f : this->frames)
    {
        f.reset();
    }

    this->pipeline_map.clear();
    this->render_pass.reset();
    this->depth_buffer.reset();
    this->swapchain.reset();

    this->initializeRenderer();
}

void gfx::Renderer::initializeRenderer()
{
    this->swapchain = std::make_shared<vulkan::Swapchain>(
        this->device, this->draw_surface, this->window.size());

    this->depth_buffer = std::make_shared<vulkan::Image2D>(
        this->allocator,
        this->device,
        this->swapchain->getExtent(),
        vk::Format::eD32Sfloat,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::ImageAspectFlagBits::eDepth,
        vk::ImageTiling::eOptimal,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    this->render_pass = std::make_shared<vulkan::RenderPass>(
        this->device, this->swapchain, this->depth_buffer);

    // Pipeline Creation!
    // TODO: replace with magic enum iter?

    std::vector<std::shared_ptr<util::Future<void>>> futures {};

    auto [sender, receiver] =
        util::mpmc::create<std::pair<vulkan::PipelineType, vulkan::Pipeline>>();

    futures.push_back(util::runAsynchronously<void>(
        [&]
        {
            sender.send(std::make_pair(
                vulkan::PipelineType::Flat,
                vulkan::createPipeline(
                    vulkan::PipelineType::Flat,
                    this->device,
                    this->render_pass,
                    this->swapchain)));
        }));

    futures.push_back(util::runAsynchronously<void>(
        [&]
        {
            sender.send(std::make_pair(
                vulkan::PipelineType::Voxel,
                vulkan::createPipeline(
                    vulkan::PipelineType::Voxel,
                    this->device,
                    this->render_pass,
                    this->swapchain)));
        }));

    for (std::shared_ptr<util::Future<void>>& f : futures)
    {
        f->await();
    }

    while (std::optional<std::pair<vulkan::PipelineType, vulkan::Pipeline>>
               receivedPipeline = receiver.receive())
    {
        this->pipeline_map[receivedPipeline->first] =
            std::move(receivedPipeline->second);
    }

    util::assertFatal(
        this->pipeline_map.size() == vulkan::PipelineTypeNumberOfValidEntries,
        "Pipelines were not correctly populated! {} {}",
        this->pipeline_map.size(),
        vulkan::PipelineTypeNumberOfValidEntries);

    std::shared_ptr<std::vector<vk::UniqueFramebuffer>> framebuffers =
        std::make_shared<std::vector<vk::UniqueFramebuffer>>();

    // TODO: abstract this into its own FrameBuffer class
    // also figure out how to make this sync the attachments with the
    // pipelines
    for (const vk::UniqueImageView& view : this->swapchain->getRenderTargets())
    {
        std::array<vk::ImageView, 2> attachments {*view, **this->depth_buffer};

        vk::FramebufferCreateInfo frameBufferCreateInfo {
            .sType {vk::StructureType::eFramebufferCreateInfo},
            .pNext {nullptr},
            .flags {},
            .renderPass {**this->render_pass},
            .attachmentCount {attachments.size()},
            .pAttachments {attachments.data()},
            .width {this->swapchain->getExtent().width},
            .height {this->swapchain->getExtent().height},
            .layers {1},
        };

        framebuffers->push_back(
            this->device->asLogicalDevice().createFramebufferUnique(
                frameBufferCreateInfo));
    }

    for (std::unique_ptr<Frame>& f : this->frames)
    {
        f = std::make_unique<Frame>(
            this->device, this->swapchain, this->render_pass, framebuffers);
    }
}

vk::Extent2D gfx::Renderer::getExtent() const
{
    return this->swapchain->getExtent();
}