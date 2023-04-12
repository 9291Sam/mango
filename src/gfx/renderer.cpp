#include "renderer.hpp"
#include "util/log.hpp"
#include "vulkan/allocator.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/device.hpp"
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
        , swapchain       {nullptr}
        , depth_buffer    {nullptr}
        , render_pass     {nullptr}
        , descriptor_pool {nullptr}
    {
        const vk::DynamicLoader         dl;
        const PFN_vkGetInstanceProcAddr dynVkGetInstanceProcAddr =
            dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"
            );
        VULKAN_HPP_DEFAULT_DISPATCHER.init(dynVkGetInstanceProcAddr);

        this->instance = std::make_unique<vulkan::Instance>(
            dynVkGetInstanceProcAddr,
            [&](vk::Instance instance_)
            {
                VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);
            }
        );

        this->draw_surface = std::make_shared<vk::UniqueSurfaceKHR>(
            this->window.createSurface(**this->instance)
        );

        this->device = std::make_shared<vulkan::Device>(
            this->instance,
            **this->draw_surface,
            [&](vk::Device device_)
            {
                VULKAN_HPP_DEFAULT_DISPATCHER.init(**this->instance, device_);
            }
        );

        this->allocator = std::make_shared<vulkan::Allocator>(
            this->instance,
            this->device,
            dynVkGetInstanceProcAddr,
            dl.getProcAddress<PFN_vkGetDeviceProcAddr>("vkGetDeviceProcAddr")
        );

        this->initializeRenderer();

        util::logLog("Renderer initialization complete");
    }

    Renderer::~Renderer() {}

    void Renderer::initializeRenderer()
    {
        this->swapchain = std::make_shared<vulkan::Swapchain>(
            this->device, this->draw_surface, this->window.size()
        );

        this->depth_buffer = std::make_shared<vulkan::Image2D>(
            this->allocator,
            this->device,
            this->swapchain->getExtent(),
            vk::Format::eD32Sfloat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment,
            vk::ImageAspectFlagBits::eDepth,
            vk::ImageTiling::eOptimal,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );

        this->render_pass = std::make_unique<vulkan::RenderPass>(
            this->device, this->swapchain, this->depth_buffer
        );

        std::unordered_map<vk::DescriptorType, std::uint32_t> descriptorMap {};
        descriptorMap[vk::DescriptorType::eUniformBuffer]        = 12;
        descriptorMap[vk::DescriptorType::eCombinedImageSampler] = 12;
        // TODO: make not magic numbers and based instead on pipelines and sizes
        // and other dynamic stuff

        this->descriptor_pool =
            vulkan::DescriptorPool::create(this->device, descriptorMap);
    }
} // namespace gfx
