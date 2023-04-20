#include "gfx/object.hpp"
#include "gfx/renderer.hpp"
#include "gfx/vulkan/gpu_data.hpp"
#include "util/log.hpp"

int main()
{
    util::logLog("mango started");

    const std::array<gfx::vulkan::Vertex, 3> vertices {
        gfx::vulkan::Vertex {
                             .position {1.0f, 1.0f, 0.0f},
                             .color {1.0f, 0.0f, 0.0f},
                             .normal {},
                             .uv {},
                             },

        gfx::vulkan::Vertex {
                             .position {-1.0f, 1.0f, 0.0f},
                             .color {0.0f, 1.0f, 0.0f},
                             .normal {},
                             .uv {},
                             },

        gfx::vulkan::Vertex {
                             .position {0.0f, -1.0f, 0.0f},
                             .color {0.0f, 0.0f, 1.0f},
                             .normal {},
                             .uv {},
                             },
    };

    try
    {
        gfx::Renderer renderer {};

        const std::unique_ptr<gfx::VertexObject> vertexObject =
            renderer.createObject(vertices);

        gfx::Object* object = dynamic_cast<gfx::Object*>(vertexObject.get());

        while (!renderer.shouldClose())
        {
            renderer.drawObject(*object);
        }
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }

    util::logLog("mango exited successfully");
}
