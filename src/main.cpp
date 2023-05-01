#include "gfx/camera.hpp"
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

        // TODO: make everything resonablly sized
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
