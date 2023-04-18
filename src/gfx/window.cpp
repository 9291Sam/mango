#include "window.hpp"
#include "vulkan/includes.hpp"
#include <thread>
#include <util/log.hpp>


// clang-format off
#include "GLFW/glfw3.h"
// clang-format on
namespace gfx
{
    Window::Window(vk::Extent2D size, const char* name)
        : window {nullptr}
        , width {static_cast<int>(size.width)}
        , height {static_cast<int>(size.height)}
        , was_resized {std::atomic_bool {false}}
    {
        if (glfwInit() != GLFW_TRUE)
        {
            util::panic("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        this->window = glfwCreateWindow(
            static_cast<int>(size.width),
            static_cast<int>(size.height),
            name,
            nullptr,
            nullptr);

        if (this->window == nullptr)
        {
            util::panic("Failed to create glfw window");
        }

        // Putting a reference to `this` inside of GLFW so that it can be passed
        // to the callback function
        glfwSetWindowUserPointer(this->window, static_cast<void*>(this));
        std::ignore = glfwSetFramebufferSizeCallback(
            this->window, Window::frameBufferResizeCallback);
    }

    Window::~Window()
    {
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

    bool Window::shouldClose() const
    {
        return static_cast<bool>(glfwWindowShouldClose(this->window));
    }

    bool Window::isKeyPressed(int key) const
    {
        if (glfwGetKey(this->window, key) == GLFW_PRESS)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    vk::Extent2D Window::size() const
    {
        // TODO: add a mutex for the width and height components together
        return vk::Extent2D {
            .width {static_cast<std::uint32_t>(this->width.load())},
            .height {static_cast<std::uint32_t>(this->height.load())}};
    }

    vk::UniqueSurfaceKHR Window::createSurface(vk::Instance instance) const
    {
        VkSurfaceKHR maybeSurface = nullptr;

        const VkResult result = glfwCreateWindowSurface(
            static_cast<VkInstance>(instance),
            this->window,
            nullptr,
            &maybeSurface);

        util::assertFatal(
            result == VK_SUCCESS,
            "Failed to create window surface | {}",
            vk::to_string(vk::Result {result}));

        util::assertFatal(
            static_cast<bool>(maybeSurface), "Returned surface was a nullptr!");

        return vk::UniqueSurfaceKHR {vk::SurfaceKHR {maybeSurface}, instance};
    }

    void Window::pollEvents() const
    {
        glfwPollEvents();
    }

    void Window::blockThisThreadWhileMinimized() const
    {
        while (this->size().width == 0 || this->size().height == 0)
        {
            glfwWaitEvents();
            std::this_thread::yield();
        }
    }

    void Window::frameBufferResizeCallback(
        GLFWwindow* glfwWindow, int newWidth, int newHeight)
    {
        gfx::Window* window =
            static_cast<gfx::Window*>(glfwGetWindowUserPointer(glfwWindow));

        window->was_resized.store(true);
        window->width.store(newWidth);
        window->height.store(newHeight);
    }

} // namespace gfx
