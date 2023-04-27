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

    const std::array<gfx::vulkan::Index, 3> indicies {0, 1, 2};

    try
    {
        gfx::Renderer renderer {};

        std::vector<std::shared_ptr<gfx::Object>> objects {};
        objects.push_back(renderer.createFlatObject(vertices, indicies));
        objects.at(0)->transform.scale *= 100;

        while (!renderer.shouldClose())
        {
            // objects.at(0)->

            std::vector<const gfx::Object*> drawingObjects {};
            drawingObjects.reserve(objects.size());

            for (const std::shared_ptr<gfx::Object>& o : objects)
            {
                drawingObjects.push_back(o.get());
            }

            renderer.drawObjects(drawingObjects);
        }
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }

    util::logLog("mango exited successfully");
}
