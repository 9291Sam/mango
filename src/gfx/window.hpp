#include "util/threads.hpp"
#include "vulkan/includes.hpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <semaphore>

struct GLFWwindow;

namespace gfx
{
    /// @brief Abstraction over a cross platform window with an easily creatable
    /// draw surface
    class Window
    {
    public:
        /// @brief Normalized 0 - 1 values accross screen space
        struct Delta
        {
            double x;
            double y;
        };
    public:

        Window(vk::Extent2D size, const char* name);
        ~Window();

        Window(const Window&)             = delete;
        Window(Window&&)                  = delete;
        Window& operator= (const Window&) = delete;
        Window& operator= (Window&&)      = delete;

        [[nodiscard]] bool         shouldClose() const;
        // TODO: write a wrapper around these GLFW_KEYS
        [[nodiscard]] bool         isKeyPressed(int) const;
        [[nodiscard]] vk::Extent2D size() const;
        // [[nodiscard]] double       getDeltaTimeSeconds() const;
        // [[nodiscard]] Delta        getMouseDelta() const;

        [[nodiscard]] vk::UniqueSurfaceKHR createSurface(vk::Instance) const;

        // void attachCursor() const;
        // void detachCursor() const;
        void pollEvents() const;

        void blockThisThreadWhileMinimized() const;
    private:
        // this needs to be here so it can access members of the class
        static void frameBufferResizeCallback(GLFWwindow*, int, int);

        GLFWwindow*                               window;
        util::Mutex<std::uint32_t, std::uint32_t> width_height;
        std::atomic<bool>                         was_resized;
    }; // class Window
} // namespace gfx
