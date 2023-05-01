#include "gfx/camera.hpp"
#include "gfx/object.hpp"
#include "gfx/renderer.hpp"
#include "gfx/vulkan/gpu_data.hpp"
#include "util/log.hpp"

int main()
{
    util::logLog("mango started");

    const std::array<gfx::vulkan::Vertex, 8> vertices {
        gfx::vulkan::Vertex {
            .position {-1.0f, -1.0f, -1.0f},
            .color {0.0f, 0.0f, 0.0f},
            .normal {},
            .uv {},
        },
        gfx::vulkan::Vertex {
            .position {-1.0f, -1.0f, 1.0f},
            .color {0.0f, 0.0f, 1.0f},
            .normal {},
            .uv {},
        },
        gfx::vulkan::Vertex {
            .position {-1.0f, 1.0f, -1.0f},
            .color {0.0f, 1.0f, 0.0f},
            .normal {},
            .uv {},
        },
        gfx::vulkan::Vertex {
            .position {-1.0f, 1.0f, 1.0f},
            .color {0.0f, 1.0f, 1.0f},
            .normal {},
            .uv {},
        },
        gfx::vulkan::Vertex {
            .position {1.0f, -1.0f, -1.0f},
            .color {1.0f, 0.0f, 0.0f},
            .normal {},
            .uv {},
        },
        gfx::vulkan::Vertex {
            .position {1.0f, -1.0f, 1.0f},
            .color {1.0f, 0.0f, 1.0f},
            .normal {},
            .uv {},
        },
        gfx::vulkan::Vertex {
            .position {1.0f, 1.0f, -1.0f},
            .color {1.0f, 1.0f, 0.0f},
            .normal {},
            .uv {},
        },
        gfx::vulkan::Vertex {
            .position {1.0f, 1.0f, 1.0f},
            .color {1.0f, 1.0f, 1.0f},
            .normal {},
            .uv {},
        },
    };

    const std::array<gfx::vulkan::Index, 36> indicies {
        2, 6, 7, 2, 3, 7, 0, 4, 5, 0, 1, 5, 0, 2, 6, 0, 4, 6,
        1, 3, 7, 1, 5, 7, 0, 2, 3, 0, 1, 3, 4, 6, 7, 4, 5, 7};

    try
    {
        gfx::Renderer renderer {};

        std::vector<gfx::Object> objects {};
        objects.push_back(renderer.createObject(
            gfx::Renderer::PipelineType::FlatPipeline, vertices, indicies));
        objects.at(0).transform.scale *= 25;

        std::vector<const gfx::Object*> drawingObjects {};
        drawingObjects.reserve(objects.size());
        for (const gfx::Object& o : objects)
        {
            drawingObjects.push_back(&o);
        }

        gfx::Camera camera {{-0.0f, 0.0f, 20.0f}};
        // camera.addPitch(-0.570792479f);
        // camera.addYaw(0.785398f);

        while (!renderer.shouldClose())
        {
            objects.at(0).transform.yawBy(
                1.0f * renderer.getDeltaTimeSeconds());

            renderer.updateCamera(camera);

            renderer.drawObjects(camera, drawingObjects);
        }
    }
    catch (const std::exception& e)
    {
        util::logFatal("Exception propagated to main {}", e.what());
    }

    util::logLog("mango exited successfully");
}
