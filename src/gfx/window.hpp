#include <chrono>
#include <memory>
#include <semaphore>

namespace vk
{
    struct Extent2D;
    struct UniqueSurfaceKHR;
    struct Instance;
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
        vkfw::

    }; // class Window
} // namespace gfx
