#ifndef SRC_GFX_VULKAN_DESCRIPTORS_HPP
#define SRC_GFX_VULKAN_DESCRIPTORS_HPP

#include "includes.hpp"
#include <expected>
#include <memory>
#include <span>
#include <unordered_map>

namespace gfx::vulkan
{
    class Device;
    class DescriptorSet;
    class DescriptorSetLayout;

    enum class DescriptorSetType
    {
        None,
        Voxel,
    };

    struct DescriptorState
    {
        DescriptorState()
            : descriptors {{
                DescriptorSetType::None,
                DescriptorSetType::None,
                DescriptorSetType::None,
                DescriptorSetType::None,
            }}
        {}

        std::array<DescriptorSetType, 4> descriptors;

        void reset()
        {
            for (auto& d : descriptors)
            {
                d = DescriptorSetType::None;
            }
        }
    };

    // class DynamicDescriptorPool
    // {
    // public:

    //     DynamicDescriptorPool(std::shared_ptr<Device>);

    //     DynamicDescriptorPool(const DynamicDescriptorPool&) = delete;
    //     DynamicDescriptorPool(DynamicDescriptorPool&&)      = delete;
    //     DynamicDescriptorPool&
    //     operator= (const DynamicDescriptorPool&)                   = delete;
    //     DynamicDescriptorPool& operator= (DynamicDescriptorPool&&) = delete;

    //     [[nodiscard]] DescriptorSet
    //         allocate(std::shared_ptr<DescriptorSetLayout>);

    // private:
    //     std::shared_ptr<Device> device;

    //     std::shared_ptr<DescriptorPool> current_pool;
    // };

    // TODO: add proper atomics to this
    class DescriptorPool : public std::enable_shared_from_this<DescriptorPool>
    {
    public:
        struct AllocationFailure
        {
            std::size_t        tried_to_allocate;
            vk::DescriptorType type;
            std::size_t        number_available;

            // TODO: operator std::string
        };
    public:

        [[nodiscard]] static std::shared_ptr<DescriptorPool> create(
            std::shared_ptr<Device>,
            std::unordered_map<vk::DescriptorType, std::uint32_t> capacity);
        ~DescriptorPool() = default;

        DescriptorPool()                                  = delete;
        DescriptorPool(const DescriptorPool&)             = delete;
        DescriptorPool(DescriptorPool&&)                  = delete;
        DescriptorPool& operator= (const DescriptorPool&) = delete;
        DescriptorPool& operator= (DescriptorPool&&)      = delete;

        // [[nodiscard]] std::expected<DescriptorSet, AllocationFailure>
        [[nodiscard]] DescriptorSet
            allocate(std::shared_ptr<DescriptorSetLayout>);

    private:

        DescriptorPool(
            std::shared_ptr<Device>,
            std::unordered_map<vk::DescriptorType, std::uint32_t> capacity);

        friend class DescriptorSet;
        void free(DescriptorSet&);

        std::shared_ptr<Device>  device;
        vk::UniqueDescriptorPool pool;
        // std::unordered_map<vk::DescriptorType, std::uint32_t>
        //     initial_descriptors;
        std::unordered_map<vk::DescriptorType, std::uint32_t>
            available_descriptors;
    }; // class DescriptorPool

    class DescriptorSetLayout
    {
    public:

        DescriptorSetLayout(
            std::shared_ptr<Device>, vk::DescriptorSetLayoutCreateInfo);
        ~DescriptorSetLayout() = default;

        DescriptorSetLayout()                                       = delete;
        DescriptorSetLayout(const DescriptorSetLayout&)             = delete;
        DescriptorSetLayout(DescriptorSetLayout&&)                  = delete;
        DescriptorSetLayout& operator= (const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator= (DescriptorSetLayout&&)      = delete;

        [[nodiscard]] const vk::DescriptorSetLayout* operator* () const;
        [[nodiscard]] auto                           getLayoutBindings() const
            -> const std::vector<vk::DescriptorSetLayoutBinding>&;
        // TODO: replace with span once intellissense fixes its shit

    private:
        std::shared_ptr<Device>                     device;
        vk::UniqueDescriptorSetLayout               layout;
        std::vector<vk::DescriptorSetLayoutBinding> descriptors;
    };

    // RAII wrapper around a vulkan descriptor set
    class DescriptorSet
    {
    public:

        ~DescriptorSet();

        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet(DescriptorSet&&);
        DescriptorSet& operator= (const DescriptorSet&) = delete;
        DescriptorSet& operator= (DescriptorSet&&);

        [[nodiscard]] vk::DescriptorSet operator* () const;

    private:
        friend class DescriptorPool;
        DescriptorSet(
            std::shared_ptr<DescriptorPool>,
            std::shared_ptr<DescriptorSetLayout>,
            vk::DescriptorSet);

        vk::DescriptorSet                    set;
        std::shared_ptr<DescriptorSetLayout> layout;
        std::shared_ptr<DescriptorPool>      pool;
    };

} // namespace gfx::vulkan

#endif // SRC_GFX_VULKAN_DESCRIPTORS_HPP
