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
        , input_ignore_frames {3}
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

        this->key_map[GLFW_KEY_W]            = Action::PlayerMoveForward;
        this->key_map[GLFW_KEY_S]            = Action::PlayerMoveBackward;
        this->key_map[GLFW_KEY_A]            = Action::PlayerMoveLeft;
        this->key_map[GLFW_KEY_D]            = Action::PlayerMoveRight;
        this->key_map[GLFW_KEY_LEFT_CONTROL] = Action::PlayerMoveDown;
        this->key_map[GLFW_KEY_SPACE]        = Action::PlayerMoveUp;
        this->key_map[GLFW_KEY_LEFT_SHIFT]   = Action::PlayerSprint;
        // this->key_map[GLFW_KEY_I]            = Action::CursorAttach;
        this->key_map[GLFW_KEY_ESCAPE]       = Action::CursorDetach;

        // Putting a reference to `this` inside of GLFW so that it can be passed
        // to the callback function
        glfwSetWindowUserPointer(this->window, static_cast<void*>(this));

        std::ignore = glfwSetFramebufferSizeCallback(
            this->window, Window::frameBufferResizeCallback);

        std::ignore =
            glfwSetKeyCallback(this->window, Window::keypressCallback);

        std::ignore = glfwSetWindowFocusCallback(
            this->window, Window::windowFocusCallback);

        std::ignore = glfwSetMouseButtonCallback(
            this->window, Window::mouseButtonCallback);

        this->last_frame_end_time = std::chrono::steady_clock::now();

        using namespace std::chrono_literals;
        this->last_frame_duration = 16ms;
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
        auto [width, height] = this->width_height.load();

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
        return this->last_frame_duration.count();
    }

    auto Window::getMouseDelta() const -> Delta
    {
        if (this->is_cursor_attached.load() && this->input_ignore_frames == 0)
        {
            return Delta {
                .x {static_cast<float>(this->mouse_delta_pixels.first)
                    / this->size().width},
                .y {static_cast<float>(this->mouse_delta_pixels.second)
                    / this->size().height},
            };
        }
        else
        {
            return Delta {.x {0.0f}, .y {0.0f}};
        }
    }

    void Window::pollEvents()
    {
        glfwPollEvents();

        if (this->input_ignore_frames > 0)
        {
            --this->input_ignore_frames;
        }

        if (this->keyboard_states
                .at(static_cast<std::size_t>(Action::CursorAttach))
                .load())
        {
            this->attachCursor();
        }

        if (this->keyboard_states
                .at(static_cast<std::size_t>(Action::CursorDetach))
                .load())
        {
            this->detachCursor();
        }

        // Mouse processing
        std::pair<double, double> currentMousePosition {
            std::nan(""), std::nan("")};

        glfwGetCursorPos(
            this->window,
            &currentMousePosition.first,
            &currentMousePosition.second);

        this->mouse_delta_pixels = {
            currentMousePosition.first - this->previous_mouse_position.first,
            currentMousePosition.second - this->previous_mouse_position.second};

        this->previous_mouse_position = currentMousePosition;

        // Delta time processing
        const auto currentTime = std::chrono::steady_clock::now();

        this->last_frame_duration = currentTime - this->last_frame_end_time;

        this->last_frame_end_time = currentTime;
    }

    void Window::attachCursor() const
    {
        glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        this->is_cursor_attached.store(true);
    }

    void Window::detachCursor() const
    {
        if (!this->is_cursor_attached.load())
        {
            return;
        }

        glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPos(
            this->window,
            static_cast<double>(this->size().width / 2),
            static_cast<double>(this->size().height / 2));

        this->is_cursor_attached.store(false);
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
        window->width_height.set(
            static_cast<std::uint32_t>(newWidth),
            static_cast<std::uint32_t>(newHeight));
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

    void Window::windowFocusCallback(GLFWwindow* glfwWindow, int isFocused)
    {
        gfx::Window* window =
            static_cast<gfx::Window*>(glfwGetWindowUserPointer(glfwWindow));

        if (isFocused)
        {
            window->attachCursor();
        }
    }

    void Window::mouseButtonCallback(
        GLFWwindow* glfwWindow, int button, int action, int)
    {
        gfx::Window* window =
            static_cast<gfx::Window*>(glfwGetWindowUserPointer(glfwWindow));

        std::atomic<bool>& buttonToModify =
            window->mouse_states.at(static_cast<std::size_t>(button));

        switch (action)
        {
        case GLFW_RELEASE:
            if (!buttonToModify.exchange(false))
            {
                util::logWarn("Mouse button #{} already released!", button);
            }
            break;

        case GLFW_PRESS:
            if (buttonToModify.exchange(true))
            {
                util::logWarn("Mouse button #{} already depressed!", button);
            }
            break;

        case GLFW_REPEAT:
            buttonToModify.store(true);
            break;

        default:
            util::panic("Unexpected action! {}", action);
        }

        // TODO: do this properly?
        // you need to switch the mapping from keys -> actions
        // into seperate keys -> actions and current action states so you can
        // just poll if the attack state is desired
        if (window->mouse_states.at(0).load()
            || window->mouse_states.at(1).load())
        {
            window->attachCursor();
        }
    }
} // namespace gfx
