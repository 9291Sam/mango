#include "window.hpp"
#include "vulkan/includes.hpp"
#include <thread>
#include <util/log.hpp>

namespace gfx
{
    Window::Window(vk::Extent2D size, const char* name)
        : window {nullptr}
        , width_height {size.width, size.height}
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

        this->key_map[GLFW_KEY_W] = Action::PlayerMoveForward;
        this->key_map[GLFW_KEY_S] = Action::PlayerMoveBackward;
        this->key_map[GLFW_KEY_A] = Action::PlayerMoveLeft;
        this->key_map[GLFW_KEY_D] = Action::PlayerMoveRight;

        // Putting a reference to `this` inside of GLFW so that it can be passed
        // to the callback function
        glfwSetWindowUserPointer(this->window, static_cast<void*>(this));
        std::ignore = glfwSetFramebufferSizeCallback(
            this->window, Window::frameBufferResizeCallback);
        std::ignore =
            glfwSetKeyCallback(this->window, Window::keypressCallback);
    }

    Window::~Window()
    {
        glfwDestroyWindow(this->window);
        glfwTerminate();
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

    vk::Extent2D Window::size() const
    {
        std::uint32_t width  = 0;
        std::uint32_t height = 0;

        this->width_height.lock(
            [&](std::uint32_t width_, std::uint32_t height_)
            {
                width  = width_;
                height = height_;
            });

        return vk::Extent2D {.width {width}, .height {height}};
    }

    bool Window::shouldClose() const
    {
        return static_cast<bool>(glfwWindowShouldClose(this->window));
    }

    bool Window::isActionActive(Action action) const
    {
        return this->keyboard_states.at(static_cast<std::size_t>(action))
            .load();
    }

    float Window::getDeltaTimeSeconds() const
    {
        util::logWarn("Invalid delta time!");
        return 1.0f;
    }

    void Window::pollEvents() const
    {
        glfwPollEvents();
    }

    void Window::blockThisThreadWhileMinimized() const
    {
        // TODO: replace with spawnin
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
        window->width_height.lock(
            [&](std::uint32_t& width, std::uint32_t& height)
            {
                width  = static_cast<std::uint32_t>(newWidth);
                height = static_cast<std::uint32_t>(newHeight);
            });
    }
    void Window::keypressCallback(
        GLFWwindow*          glfwWindow,
        int                  key,
        [[maybe_unused]] int scancode,
        int                  action,
        [[maybe_unused]] int mods)
    {
        gfx::Window* window =
            static_cast<gfx::Window*>(glfwGetWindowUserPointer(glfwWindow));

        if (!window->key_map.contains(key))
        {
            util::logTrace("Unknown key pressed {}", key);
            return;
        }

        std::atomic<bool>& keyToModify = window->keyboard_states.at(
            static_cast<std::size_t>(window->key_map.at(key)));

        switch (action)
        {
        case GLFW_RELEASE:
            if (!keyToModify.exchange(false))
            {
                util::logWarn(
                    "Key already depressed: {} | {}",
                    key,
                    static_cast<std::size_t>(window->key_map.at(key)));
            }
            break;

        case GLFW_PRESS:
            if (keyToModify.exchange(true))
            {
                util::logWarn(
                    "Key already pressed! Key: {} | {}",
                    key,
                    static_cast<std::size_t>(window->key_map.at(key)));
            }
            break;

        case GLFW_REPEAT:
            keyToModify.store(true);
            break;

        default:
            util::panic("Unexpected action! {}", action);
        }
    }

} // namespace gfx
