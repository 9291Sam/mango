#include "renderer.hpp"
#include "util/log.hpp"
#include "vulkan/allocator.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/data.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/device.hpp"
#include "vulkan/frame.hpp"
#include "vulkan/image.hpp"
#include "vulkan/includes.hpp"
#include "vulkan/pipeline.hpp"
#include "vulkan/render_pass.hpp"
#include "vulkan/swapchain.hpp"

namespace gfx
{
    Renderer::Renderer()
        : window          {{1'200, 1'200}, "Mango"}
        , instance        {nullptr}
        , draw_surface    {nullptr}
        , device          {nullptr}
        , allocator       {nullptr}
        , descriptor_pool {nullptr}
        , swapchain       {nullptr}
        , depth_buffer    {nullptr}
        , render_pass     {nullptr}
        , flat_pipeline   {nullptr}
        , render_index    {0}
        , command_pool    {nullptr}
        , framebuffers    {}
        , frames          {nullptr}
    {
        const vk::DynamicLoader         dl;
        const PFN_vkGetInstanceProcAddr dynVkGetInstanceProcAddr =
            dl.getProcAddress<PFN_vkGetInstanceProcAddr>(
                "vkGetInstanceProcAddr");
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

        // TODO: replace with a dynamic reallocating descriptor pool abstraction
        // so that this can be self sufficent and not based on magic numbers
        std::unordered_map<vk::DescriptorType, std::uint32_t> descriptorMap {};
        descriptorMap[vk::DescriptorType::eUniformBuffer]        = 12;
        descriptorMap[vk::DescriptorType::eCombinedImageSampler] = 12;

        this->descriptor_pool = vulkan::DescriptorPool::create(
            this->device, std::move(descriptorMap));

        this->initializeRenderer();

        util::logLog("Renderer initialization complete");
    }

    Renderer::~Renderer()
    {
        this->device->asLogicalDevice().waitIdle();
    }

    bool Renderer::shouldClose() const
    {
        return this->window.shouldClose();
    }

    void Renderer::drawFrame()
    {
        this->render_index = (this->render_index + 1) % this->MaxFramesInFlight;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
        static vulkan::StagedBuffer vertexBuffer {
            [this]
            {
                std::array<vulkan::Vertex, 3> vertices {
                    vulkan::Vertex {
                                    .position {1.0f, 1.0f, 0.0f},
                                    .color {1.0f, 0.0f, 0.0f},
                                    .normal {},
                                    .uv {},
                                    },

                    vulkan::Vertex {
                                    .position {-1.0f, 1.0f, 0.0f},
                                    .color {0.0f, 1.0f, 0.0f},
                                    .normal {},
                                    .uv {},
                                    },

                    vulkan::Vertex {
                                    .position {0.0f, -1.0f, 0.0f},
                                    .color {0.0f, 0.0f, 1.0f},
                                    .normal {},
                                    .uv {},
                                    },
                };

                vulkan::StagedBuffer temp {
                    *this->device,
                    this->allocator,
                    sizeof(vulkan::Vertex) * vertices.size(),
                    vk::BufferUsageFlagBits::eVertexBuffer};

                temp.write(
                    {reinterpret_cast<std::byte*>(vertices.data()),
                     sizeof(vulkan::Vertex) * vertices.size()});

                return temp;
            }()};
#pragma clang diagnostic pop

        // util::logTrace("Rendering at index {}", this->render_index);

        if (this->frames.at(this->render_index)
                ->render(
                    this->framebuffers, *this->flat_pipeline, vertexBuffer))
        {
            util::panic("Resize requested!");
        }

        this->window.pollEvents();
    }

    void Renderer::initializeRenderer()
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

        this->flat_pipeline = std::make_unique<vulkan::FlatPipeline>(
            this->device, this->swapchain, this->render_pass);

        // TODO: :vomit:
        vk::CommandPoolCreateInfo commandPoolCreateInfo {
            .sType {vk::StructureType::eCommandPoolCreateInfo},
            .pNext {nullptr},
            .flags {vk::CommandPoolCreateFlagBits::eResetCommandBuffer},
            .queueFamilyIndex {this->device->getQueueFamilyIndex()}};

        this->command_pool =
            this->device->asLogicalDevice().createCommandPoolUnique(
                commandPoolCreateInfo);

        const vk::CommandBufferAllocateInfo commandBuffersAllocateInfo {
            .sType {vk::StructureType::eCommandBufferAllocateInfo},
            .pNext {},
            .commandPool {*this->command_pool},
            .level {vk::CommandBufferLevel::ePrimary},
            .commandBufferCount {
                static_cast<std::uint32_t>(this->MaxFramesInFlight)},
        };

        auto commandBufferVector =
            this->device->asLogicalDevice().allocateCommandBuffersUnique(
                commandBuffersAllocateInfo);

        for (std::size_t i = 0; i < this->MaxFramesInFlight; ++i)
        {
            this->frames.at(i) = std::make_unique<vulkan::Frame>(
                this->device,
                this->swapchain,
                this->render_pass,
                std::move(commandBufferVector.at(i)));
        }

        for (const vk::UniqueImageView& view :
             this->swapchain->getRenderTargets())
        {
            // TODO: abstract this away into a new FrameBuffer class
            // also figure out a way to make this synced with the pipelines?
            std::array<vk::ImageView, 2> attachments {
                *view, **this->depth_buffer};

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

            this->framebuffers.push_back(
                this->device->asLogicalDevice().createFramebufferUnique(
                    frameBufferCreateInfo));
        }
    }
} // namespace gfx
