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

        [[nodiscard]] auto createSurface(vk::Instance) const
            -> vk::UniqueSurfaceKHR;

        // void attachCursor() const;
        // void detachCursor() const;
        void pollEvents() const;

        // TODO: replace with a semaphore + spawning another thread?
        // void blockThisThreadWhileMinimized() const;
    private:
        // this needs to be here so it can access members of the class
        static void frameBufferResizeCallback(GLFWwindow*, int, int);

        GLFWwindow*       window;
        std::atomic<int>  width;
        std::atomic<int>  height;
        std::atomic<bool> was_resized;
        std::byte         _padding[7];
    }; // class Window
} // namespace gfx
