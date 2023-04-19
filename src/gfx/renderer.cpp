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
        , device          {nullptr}
        , allocator       {nullptr}
        , descriptor_pool {nullptr}
        , swapchain       {nullptr}
        , depth_buffer    {nullptr}
        , render_pass     {nullptr}
        , flat_pipeline   {nullptr}
        , render_index    {0}
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

        // TODO: make dynamic auto reallocing class
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

        if (this->frames.at(this->render_index)
                ->render(
                    this->framebuffers, *this->flat_pipeline, vertexBuffer))
        {
            this->resize();
        }

        this->window.pollEvents();
    }

    void Renderer::resize()
    {
        this->window.blockThisThreadWhileMinimized();
        this->device->asLogicalDevice().waitIdle();

        for (std::unique_ptr<vulkan::Frame>& f : this->frames)
        {
            f.reset();
        }

        this->framebuffers.clear();
        this->flat_pipeline.reset();
        this->render_pass.reset();
        this->depth_buffer.reset();
        this->swapchain.reset();

        this->initializeRenderer();
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

        // TODO: abstract this into its own FrameBuffer class
        // also figure out how to make this sync the attachments with the
        // pipelines
        for (const vk::UniqueImageView& view :
             this->swapchain->getRenderTargets())
        {
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

        for (std::unique_ptr<vulkan::Frame>& f : this->frames)
        {
            f = std::make_unique<vulkan::Frame>(
                this->device, this->swapchain, this->render_pass);
        }
    }
} // namespace gfx
