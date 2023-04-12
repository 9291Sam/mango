#include "descriptors.hpp"
#include "device.hpp"
#include "util/log.hpp"

namespace gfx::vulkan
{
    std::shared_ptr<DescriptorPool> DescriptorPool::create(
        std::shared_ptr<Device>                               device,
        std::unordered_map<vk::DescriptorType, std::uint32_t> capacity
    )
    {
        // We need to access a private constructor, std::make_shared won't work
        return std::shared_ptr<DescriptorPool>(new DescriptorPool {
            std::move(device), std::move(capacity)});
    }

    DescriptorSet
    DescriptorPool::allocate(std::shared_ptr<DescriptorSetLayout> layout)
    {
        // ensure we have enough descriptors available for the desired
        // descriptor set
        for (const auto& binding : layout->getLayoutBindings())
        {
            if (this->available_descriptors[binding.descriptorType]
                < binding.descriptorCount)
            {
                util::panic(
                    "Unable to allocate {} descriptors of type {} from a pool "
                    "with only {} available!",
                    binding.descriptorCount,
                    vk::to_string(binding.descriptorType),
                    this->available_descriptors[binding.descriptorType]
                );
            }
        }

        // the allocation will succeed decrement internal counts
        for (const auto& binding : layout->getLayoutBindings())
        {
            this->available_descriptors[binding.descriptorType] -=
                binding.descriptorCount;
        }

        const vk::DescriptorSetAllocateInfo descriptorAllocationInfo {

            .sType {vk::StructureType::eDescriptorSetAllocateInfo},
            .pNext {nullptr},
            .descriptorPool {*this->pool},
            .descriptorSetCount {1}, // TODO: add array function?
            .pSetLayouts {**layout},
        };

        // TODO: create an array function to deal with multiple allocation
        std::vector<vk::DescriptorSet> allocatedDescriptors =
            this->device->asLogicalDevice().allocateDescriptorSets(
                descriptorAllocationInfo
            );

        util::assertFatal(
            allocatedDescriptors.size() == 1,
            "Invalid descriptor length returned"
        );

        return DescriptorSet {
            this->shared_from_this(),
            std::move(layout),
            allocatedDescriptors.at(0)};
    }

    DescriptorPool::DescriptorPool(
        std::shared_ptr<Device>                               device_,
        std::unordered_map<vk::DescriptorType, std::uint32_t> capacity
    )
        : device {std::move(device_)}
        , pool {nullptr}
        , available_descriptors {std::move(capacity)}
    {
        this->available_descriptors.bucket_size(32);

        std::vector<vk::DescriptorPoolSize> requestedPoolMembers {};

        for (auto [descriptor, number] : this->available_descriptors)
        {
            util::logTrace("Number: {}", number);

            requestedPoolMembers.push_back(vk::DescriptorPoolSize {
                .type {descriptor}, .descriptorCount {number}});
        }

        util::logTrace(
            "descriptor pool size : {}", requestedPoolMembers.size()
        );

        const vk::DescriptorPoolCreateInfo poolCreateInfo {
            .sType {vk::StructureType::eDescriptorPoolCreateInfo},
            .pNext {nullptr},
            .flags {vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet},
            .maxSets {4}, // why the **fuckk** is this needed
            .poolSizeCount {
                static_cast<std::uint32_t>(requestedPoolMembers.size())},
            .pPoolSizes {requestedPoolMembers.data()},
        };

        this->pool = this->device->asLogicalDevice().createDescriptorPoolUnique(
            poolCreateInfo
        );
    }

    void DescriptorPool::free(DescriptorSet& setToFree)
    {
        for (const auto& binding : setToFree.layout->getLayoutBindings())
        {
            this->available_descriptors[binding.descriptorType] +=
                binding.descriptorCount;
        }

        // TODO: add array function
        this->device->asLogicalDevice().freeDescriptorSets(
            *this->pool, *setToFree
        );
    }

    DescriptorSetLayout::DescriptorSetLayout(
        std::shared_ptr<Device>           device_,
        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo
    )
        : device {std::move(device_)}
        , layout {nullptr}
        , descriptors {}
    {
        this->descriptors.reserve(layoutCreateInfo.bindingCount);

        for (std::size_t i = 0; i < layoutCreateInfo.bindingCount; ++i)
        {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
            this->descriptors.push_back(layoutCreateInfo.pBindings[i]);
#pragma clang diagnostic pop
        }

        this->layout =
            this->device->asLogicalDevice().createDescriptorSetLayoutUnique(
                layoutCreateInfo
            );
    }

    const vk::DescriptorSetLayout* DescriptorSetLayout::operator* () const
    {
        return &*this->layout;
    }

    const std::vector<vk::DescriptorSetLayoutBinding>&
    DescriptorSetLayout::getLayoutBindings() const
    {
        return this->descriptors;
    }

    DescriptorSet::~DescriptorSet()
    {
        if (static_cast<bool>(this->set))
        {
            this->pool->free(*this);
        }
    }

    DescriptorSet::DescriptorSet(DescriptorSet&& other)
        : set {other.set}
        , layout {std::move(other.layout)}
        , pool {std::move(other.pool)}
    {
        other.pool = nullptr;
        other.set  = nullptr;
    }

    DescriptorSet& DescriptorSet::operator= (DescriptorSet&& other)
    {
        if (&other == this)
        {
            return *this;
        }

        this->pool   = std::move(other.pool);
        this->layout = std::move(other.layout);
        this->set    = std::move(other.set);

        other.pool = nullptr;
        other.set  = nullptr;

        return *this;
    }

    vk::DescriptorSet DescriptorSet::operator* () const
    {
        return this->set;
    }

    DescriptorSet::DescriptorSet(
        std::shared_ptr<DescriptorPool>      pool_,
        std::shared_ptr<DescriptorSetLayout> layout_,
        vk::DescriptorSet                    set_
    )
        : set {set_}
        , layout {std::move(layout_)}
        , pool {std::move(pool_)}
    {
        util::logTrace(
            "Allocated descriptor set at {}", static_cast<void*>(this->set)
        );
    }
} // namespace gfx::vulkan
