#include "gfx/camera.hpp"
#include "gfx/object.hpp"
#include "gfx/renderer.hpp"
#include "gfx/vulkan/gpu_data.hpp"
#include "util/log.hpp"

std::vector<const gfx::Object*>
getPointerVector(const std::vector<std::shared_ptr<gfx::Object>> objects)
{
    std::vector<const gfx::Object*> tempObjects {};
    tempObjects.reserve(objects.size());

    for (const std::shared_ptr<gfx::Object>& o : objects)
    {
        tempObjects.push_back(o.get());
    }

    return tempObjects;
}

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
        std::vector<std::shared_ptr<gfx::Object>> objects {};
        objects.push_back(renderer.createFlatObject(vertices, indicies));
        objects.at(0)->transform.scale *= 1000;
        objects.at(0)->transform.rotation = {1.0f, 0.0f, 0.0f, 0.0f};
        objects.at(0)->transform.yawBy(0.1f);

        auto drawingObjects = getPointerVector(objects);

        gfx::Camera camera {{-0.0f, 0.0f, 2250.0f}};
        // camera.addPitch(-0.570792479f);
        // camera.addYaw(0.785398f);

        while (!renderer.shouldClose())
        {
            objects.at(0)->transform.yawBy(
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
