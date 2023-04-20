#include "pipeline.hpp"
#include "device.hpp"
#include "gpu_data.hpp"
#include "render_pass.hpp"
#include "swapchain.hpp"
#include "util/log.hpp"
#include <fstream>
#include <memory>


namespace gfx::vulkan
{
    vk::UniqueShaderModule
    Pipeline::createShaderFromFile(vk::Device device, const char* filePath)
    {
        std::ifstream fileStream {
            filePath, std::ios::in | std::ios::binary | std::ios::ate};

        util::assertFatal(
            fileStream.is_open(), "Failed to open file |{}|", filePath);

        std::vector<char> charBuffer {};
        charBuffer.resize(static_cast<std::size_t>(fileStream.tellg()));

        fileStream.seekg(0);
        fileStream.read(
            charBuffer.data(), static_cast<std::streamsize>(charBuffer.size()));

        // intellectual exercise :)
        std::vector<std::uint32_t> u32Buffer {};
        u32Buffer.resize((charBuffer.size() / 4) + 1);

        std::memcpy(u32Buffer.data(), charBuffer.data(), charBuffer.size());

        vk::ShaderModuleCreateInfo shaderCreateInfo {
            .sType {vk::StructureType::eShaderModuleCreateInfo},
            .pNext {nullptr},
            .flags {},
            .codeSize {charBuffer.size()}, // lol
            .pCode {u32Buffer.data()},
        };

        return device.createShaderModuleUnique(shaderCreateInfo);
    }

    Pipeline::Pipeline(
        std::shared_ptr<Device>                            device_,
        std::shared_ptr<Swapchain>                         swapchain_,
        std::shared_ptr<RenderPass>                        renderPass,
        std::span<const vk::PipelineShaderStageCreateInfo> shaderStages,
        vk::UniquePipelineLayout                           layout_)
        : device {std::move(device_)}
        , swapchain {std::move(swapchain_)}
        , render_pass {std::move(renderPass)}
        , layout {std::move(layout_)}
        , pipeline {nullptr}
    {
        const vk::PipelineVertexInputStateCreateInfo pipeVertexCreateInfo {
            .sType {vk::StructureType::ePipelineVertexInputStateCreateInfo},
            .pNext {nullptr},
            .flags {},
            .vertexBindingDescriptionCount {1},
            .pVertexBindingDescriptions {Vertex::getBindingDescription()},
            .vertexAttributeDescriptionCount {static_cast<std::uint32_t>(
                Vertex::getAttributeDescriptions()->size())},
            .pVertexAttributeDescriptions {
                Vertex::getAttributeDescriptions()->data()},
        };

        const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo {
            .sType {vk::StructureType::ePipelineInputAssemblyStateCreateInfo},
            .pNext {},
            .flags {},
            .topology {vk::PrimitiveTopology::eTriangleList},
            .primitiveRestartEnable {false},
        };

        const vk::Viewport viewport {
            .x {0.0f},
            .y {0.0f},
            .width {static_cast<float>(this->swapchain->getExtent().width)},
            .height {static_cast<float>(this->swapchain->getExtent().height)},
            .minDepth {0.0f},
            .maxDepth {1.0f},
        };

        const vk::Rect2D scissor {
            .offset {0, 0},
             .extent {this->swapchain->getExtent()}
        };

        const vk::PipelineViewportStateCreateInfo viewportState {
            .sType {vk::StructureType::ePipelineViewportStateCreateInfo},
            .pNext {nullptr},
            .flags {},
            .viewportCount {1},
            .pViewports {&viewport},
            .scissorCount {1},
            .pScissors {&scissor},
        };

        const vk::PipelineRasterizationStateCreateInfo rasterizationCreateInfo {
            .sType {vk::StructureType::ePipelineRasterizationStateCreateInfo},
            .pNext {nullptr},
            .flags {},
            .depthClampEnable {false},
            .rasterizerDiscardEnable {false},
            .polygonMode {vk::PolygonMode::eFill},
            .cullMode {vk::CullModeFlagBits::eNone},
            .frontFace {vk::FrontFace::eCounterClockwise},
            .depthBiasEnable {false},
            .depthBiasConstantFactor {0.0f},
            .depthBiasClamp {0.0f},
            .depthBiasSlopeFactor {0.0f},
            .lineWidth {1.0f},
        };

        const vk::PipelineMultisampleStateCreateInfo
            multiSampleStateCreateInfo {
                .sType {vk::StructureType::ePipelineMultisampleStateCreateInfo},
                .pNext {nullptr},
                .flags {},
                .rasterizationSamples {vk::SampleCountFlagBits::e1},
                .sampleShadingEnable {false},
                .minSampleShading {1.0f},
                .pSampleMask {nullptr},
                .alphaToCoverageEnable {false},
                .alphaToOneEnable {false},
            };

        const vk::PipelineColorBlendAttachmentState colorBlendAttachment {
            .blendEnable {false},
            .srcColorBlendFactor {vk::BlendFactor::eZero},
            .dstColorBlendFactor {vk::BlendFactor::eOne},
            .colorBlendOp {vk::BlendOp::eAdd},
            .srcAlphaBlendFactor {vk::BlendFactor::eOne},
            .dstAlphaBlendFactor {vk::BlendFactor::eZero},
            .alphaBlendOp {vk::BlendOp::eAdd},
            .colorWriteMask {
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                | vk::ColorComponentFlagBits::eB
                | vk::ColorComponentFlagBits::eA},
        };

        const vk::PipelineColorBlendStateCreateInfo colorBlendCreateInfo {
            .sType {vk::StructureType::ePipelineColorBlendStateCreateInfo},
            .pNext {nullptr},
            .flags {},
            .logicOpEnable {false},
            .logicOp {vk::LogicOp::eCopy},
            .attachmentCount {1},
            .pAttachments {&colorBlendAttachment},
            .blendConstants {{0.0f, 0.0f, 0.0f, 0.0f}},
        };

        vk::PipelineDepthStencilStateCreateInfo depthStencilActivator {
            .sType {vk::StructureType::ePipelineDepthStencilStateCreateInfo},
            .pNext {nullptr},
            .flags {},
            .depthTestEnable {true},
            .depthWriteEnable {true},
            .depthCompareOp {vk::CompareOp::eLess},
            .depthBoundsTestEnable {false},
            .stencilTestEnable {false},
            .front {},
            .back {},
            .minDepthBounds {0.0f},
            .maxDepthBounds {1.0f},
        };

        vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo {
            .sType {vk::StructureType::eGraphicsPipelineCreateInfo},
            .pNext {nullptr},
            .flags {},
            .stageCount {static_cast<std::uint32_t>(shaderStages.size())},
            .pStages {shaderStages.data()},
            .pVertexInputState {&pipeVertexCreateInfo},
            .pInputAssemblyState {&inputAssemblyCreateInfo},
            .pTessellationState {nullptr},
            .pViewportState {&viewportState},
            .pRasterizationState {&rasterizationCreateInfo},
            .pMultisampleState {&multiSampleStateCreateInfo},
            .pDepthStencilState {&depthStencilActivator},
            .pColorBlendState {&colorBlendCreateInfo},
            .pDynamicState {nullptr},
            .layout {*this->layout},
            .renderPass {**this->render_pass},
            .subpass {0},
            .basePipelineHandle {nullptr},
            .basePipelineIndex {-1},
        };

        auto [result, maybeGraphicsPipeline] =
            this->device->asLogicalDevice().createGraphicsPipelineUnique(
                nullptr, graphicsPipelineCreateInfo);

        util::assertFatal(
            result == vk::Result::eSuccess,
            "Failed to create graphics pipeline | Error: {}",
            vk::to_string(result));

        this->pipeline = std::move(maybeGraphicsPipeline);
    }

    vk::Pipeline Pipeline::operator* () const
    {
        return *this->pipeline;
    }
    vk::PipelineLayout Pipeline::getLayout() const
    {
        return *this->layout;
    }

    FlatPipeline::FlatPipeline(
        std::shared_ptr<Device>     device,
        std::shared_ptr<Swapchain>  swapchain,
        std::shared_ptr<RenderPass> renderPass)
        : pipeline {nullptr}
    {
        // the lifetime of this is that the must outlive the pipeline creation
        const vk::UniqueShaderModule flatVertex =
            vulkan::Pipeline::createShaderFromFile(
                device->asLogicalDevice(),
                "src/gfx/vulkan/shaders/flat_pipeline.vert.bin");

        const vk::UniqueShaderModule flatFragment =
            vulkan::Pipeline::createShaderFromFile(
                device->asLogicalDevice(),
                "src/gfx/vulkan/shaders/flat_pipeline.frag.bin");

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

        vk::UniquePipelineLayout layout =
            device->asLogicalDevice().createPipelineLayoutUnique(
                vk::PipelineLayoutCreateInfo {
                    .sType {vk::StructureType::ePipelineLayoutCreateInfo},
                    .pNext {nullptr},
                    .flags {},
                    .setLayoutCount {0},
                    .pSetLayouts {nullptr},
                    .pushConstantRangeCount {1},
                    .pPushConstantRanges {&pushConstantsInformation},
                });

        this->pipeline = std::make_unique<vulkan::Pipeline>(
            std::move(device),
            std::move(swapchain),
            std::move(renderPass),
            flatPipelineShaders,
            std::move(layout));
    }

    vk::Pipeline FlatPipeline::operator* () const
    {
        return **this->pipeline;
    }
    vk::PipelineLayout FlatPipeline::getLayout() const
    {
        return this->pipeline->getLayout();
    }
} // namespace gfx::vulkan
