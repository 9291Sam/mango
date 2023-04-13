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

        // PRe-state pipelines

        std::unordered_map<vk::DescriptorType, std::uint32_t> descriptorMap {};
        descriptorMap[vk::DescriptorType::eUniformBuffer]        = 12;
        descriptorMap[vk::DescriptorType::eCombinedImageSampler] = 12;
        // TODO: make not magic numbers and based instead on pipelines and sizes
        // and other dynamic stuff

        this->descriptor_pool = vulkan::DescriptorPool::create(
            this->device, std::move(descriptorMap)
        );

        // allocate pipelines with reference to pool

        // the lifetime of this is that the must outlive the pipeline cration
        // step
        const vk::UniqueShaderModule flatVertex =
            vulkan::Pipeline::createShaderFromFile(
                this->device->asLogicalDevice(),
                "src/gfx/vulkan/shaders/flat_pipeline.vert.bin"
            );

        const vk::UniqueShaderModule flatFragment =
            vulkan::Pipeline::createShaderFromFile(
                this->device->asLogicalDevice(),
                "src/gfx/vulkan/shaders/flat_pipeline.frag.bin"
            );

        // clang-format off
        const std::array<vk::PipelineShaderStageCreateInfo, 2>
        flatPipelineShaders
        {
            vk::PipelineShaderStageCreateInfo
            {
                .sType {vk::StructureType::ePipelineShaderStageCreateInfo},
                .pNext {nullptr},
                .flags {},
                .stage {vk::ShaderStageFlagBits::eVertex},
                .module {*flatVertex},
                .pName {"main"},
                .pSpecializationInfo {nullptr},
            },
            vk::PipelineShaderStageCreateInfo
            {
                .sType {vk::StructureType::ePipelineShaderStageCreateInfo},
                .pNext {nullptr},
                .flags {},
                .stage {vk::ShaderStageFlagBits::eFragment},
                .module {*flatFragment},
                .pName {"main"},
                .pSpecializationInfo {nullptr},
            },
        };
        // clang-format on

        const vk::PushConstantRange pushConstantsInformation {
            .stageFlags {vk::ShaderStageFlagBits::eAllGraphics},
            .offset {0},
            .size {sizeof(vulkan::PushConstants)},
        };

        // TODO: bad design move out of here
        this->flat_pipeline = std::make_unique<vulkan::Pipeline>(
            this->device,
            this->swapchain,
            this->render_pass,
            flatPipelineShaders,
            this->device->asLogicalDevice().createPipelineLayoutUnique(
                vk::PipelineLayoutCreateInfo {
                    .sType {vk::StructureType::ePipelineLayoutCreateInfo},
                    .pNext {nullptr},
                    .flags {},
                    .setLayoutCount {0},
                    .pSetLayouts {nullptr},
                    .pushConstantRangeCount {1},
                    .pPushConstantRanges {&pushConstantsInformation},
                }
            )
        )
    }
} // namespace gfx
