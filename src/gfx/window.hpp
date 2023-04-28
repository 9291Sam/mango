#include "util/threads.hpp"
#include "vulkan/includes.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <memory>
#include <semaphore>

struct GLFWwindow;

namespace gfx
{
    /// @brief Abstraction over a cross platform window with an easily creatable
    /// draw surface
    ///
    /// Because of glfw implementation details this must also manage the
    /// keyboard
    class Window
    {
    public:
        /// @brief Normalized 0 - 1 values accross screen space
        struct Delta
        {
            float x;
            float y;
        };

        enum class Action : std::uint_fast8_t // :bike:
        {
            PlayerMoveForward  = 0,
            PlayerMoveBackward = 1,
            PlayerMoveLeft     = 2,
            PlayerMoveRight    = 3,
            MaxEnumValue       = 4,
        };

        static constexpr std::size_t ActionMaxValue =
            static_cast<std::size_t>(Action::MaxEnumValue);

        using GlfwKeyType = int;
    public:

        Window(vk::Extent2D size, const char* name);
        ~Window();

        Window(const Window&)             = delete;
        Window(Window&&)                  = delete;
        Window& operator= (const Window&) = delete;
        Window& operator= (Window&&)      = delete;

        [[nodiscard]] vk::UniqueSurfaceKHR createSurface(vk::Instance) const;
        [[nodiscard]] vk::Extent2D         size() const;

        [[nodiscard]] bool shouldClose() const;

        [[nodiscard]] bool  isActionActive(Action) const;
        [[nodiscard]] float getDeltaTimeSeconds() const;
        [[nodiscard]] Delta getMouseDelta() const;

        // void attachCursor() const;
        // void detachCursor() const;
        void pollEvents() const;

        void blockThisThreadWhileMinimized() const;
    private:
        static void frameBufferResizeCallback(GLFWwindow*, int, int);
        static void keypressCallback(GLFWwindow*, int, int, int, int);

        GLFWwindow*                                   window;
        std::array<std::atomic<bool>, ActionMaxValue> keyboard_states;
        std::unordered_map<GlfwKeyType, Action>       key_map;
        util::Mutex<std::uint32_t, std::uint32_t>     width_height;
        std::atomic<bool>                             was_resized;
    }; // class Window
} // namespace gfx
