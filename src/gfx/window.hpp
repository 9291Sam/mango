#include "vulkan/includes.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <unordered_map>

struct GLFWwindow;

namespace gfx
{
    /// @brief Abstraction over a cross platform window with an easily creatable
    /// draw surface
    ///
    /// Because of glfw implementation details this must do a lot of disparte
    /// things
    /// TODO: this is becoming a mess. Now that you know everything this class
    /// needs to do, go back design it properly and dont live with your bad
    /// choices
    class Window
    {
    public:
        /// @brief Normalized 0 - 1 values across screen space
        struct Delta
        {
            float x;
            float y;
        };

        class AtomicSize
        {
        public:
            AtomicSize(std::uint32_t width, std::uint32_t height)
                : width_height {std::atomic<std::uint64_t>(0)}
            {
                this->set(width, height);
            }

            AtomicSize(const AtomicSize&)             = delete;
            AtomicSize(AtomicSize&&)                  = delete;
            AtomicSize& operator= (const AtomicSize&) = delete;
            AtomicSize& operator= (AtomicSize&&)      = delete;

            [[nodiscard]] std::pair<std::uint32_t, std::uint32_t> load() const
            {
                std::uint64_t copy = this->width_height.load();

                return std::make_pair(
                    static_cast<std::uint32_t>(copy >> 32),
                    static_cast<std::uint32_t>(copy));
            }

            void set(std::uint32_t width, std::uint32_t height)
            {
                this->width_height.store(
                    (static_cast<std::uint64_t>(width) << 32)
                    | (static_cast<std::uint64_t>(height)));
            }

        private:
            std::atomic<std::uint64_t> width_height;
        };

        enum class Action : std::uint_fast8_t // :bike:
        {
            PlayerMoveForward  = 0,
            PlayerMoveBackward = 1,
            PlayerMoveLeft     = 2,
            PlayerMoveRight    = 3,
            PlayerMoveUp       = 5,
            PlayerMoveDown     = 6,
            PlayerSprint       = 7,
            CursorAttach       = 8,
            CursorDetach       = 9,
            PrintLocation      = 10,
            MaxEnumValue       = 11,
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

        void attachCursor() const;
        void detachCursor() const;
        void pollEvents();

        void blockThisThreadWhileMinimized() const;
    private:
        static void frameBufferResizeCallback(GLFWwindow*, int, int);
        static void keypressCallback(GLFWwindow*, int, int, int, int);
        static void windowFocusCallback(GLFWwindow*, int);
        static void mouseButtonCallback(GLFWwindow*, int, int, int);

        GLFWwindow* window;

        // Frame Times
        std::chrono::time_point<std::chrono::steady_clock> last_frame_end_time;
        std::chrono::duration<float>                       last_frame_duration;

        // Window
        AtomicSize        width_height;
        std::atomic<bool> was_resized;

        std::size_t input_ignore_frames;

        // Keyboard
        std::array<std::atomic<bool>, ActionMaxValue> keyboard_states;
        std::array<std::atomic<bool>, 8>              mouse_states;
        std::unordered_map<GlfwKeyType, Action>       key_map;

        // Mouse
        std::pair<double, double> previous_mouse_position;
        std::pair<double, double> mouse_delta_pixels;
        mutable std::atomic<bool> is_cursor_attached;

    }; // class Window
} // namespace gfx
