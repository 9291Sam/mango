#include <chrono>
#include <memory>
#include <semaphore>

struct GLFWwindow;

namespace vk
{
    template<class T, class Y>
    class UniqueHandle;

    class SurfaceKHR;
    class DispatchLoaderDynamic;

    struct Extent2D;
    using UniqueSurfaceKHR = UniqueHandle<SurfaceKHR, DispatchLoaderDynamic>;
    class Instance;
} // namespace vk

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
        [[nodiscard]] bool         isKeyPressed() const;
        [[nodiscard]] vk::Extent2D size() const;
        [[nodiscard]] double       getDeltaTimeSeconds() const;
        [[nodiscard]] Delta        getMouseDelta() const;

        [[nodiscard]] auto createSurface(vk::Instance) const
            -> vk::UniqueSurfaceKHR;

        void attachCursor() const;
        void detachCursor() const;
        void pollEvents() const;

        // TODO: replace with a semaphore + spawning another thread?
        void blockThisThreadWhileMinimized() const;

    private:
        GLFWwindow* window;
    }; // class Window
} // namespace gfx
