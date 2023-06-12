## Demonstration video

[![perlin_noise_test.mp4](perlin_noise_test.mp4)](perlin_noise_test.mp4)

8192^3 voxel volume drawing about ~67 million Voxels in realtime

## Plans for the future

Graphics optimization chages:
Re-do the piopeline + object abstraction'
template pipelines over Vertex Type

Many differing pipelines wont be needed. just make them all at once up front and just store them


Object
 - Buffers
 - pipeline(enum)

 SpecializedObject : Object
 { // automatically grabs the right pipeline
    holds buffers + all the bits that are required
 }

 Say the world wants to render some stuff it creates a World




Voxel tree re-write!!!
Goals:
Voxel Optimizations
Modification 
Vertex optimizations (position + sRGBA color) 16 bytes (12 for position, 4 for srgbA)
LODs (re write)
Loading in/out

floating origin (later)


Graphics changes:
Ambient Occlussion
Voxel spread lighting


Proper world:
Multiple types of voxels
(statc map of Voxel -> GraphicsVoxel i.e sRGB)
diggable
Player physics (plan for more general physics)
(the goal of magic like stuff but its actually technology)

Debug menu



gameplay?????




Long term:
Replace gcem with cmath once its constexpr