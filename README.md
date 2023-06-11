## Demonstration video

[![std_atan2.mp4](std_atan2.mp4)](std_atan2.mp4)

8192^3 voxel volume drawing about ~80 million voxels in realtime

TODO:

Figure out a better type of noise 

Put this in the world generator (seeded)

allow for "all voxels of same type optimization" (solid world)
generate the world with multiple objects
allow for modification
multiple "octrees" that can be loaded in and out
iffering levels of detail if its far away (approximation for hihher levels by reading only parts of the tree when its far)
 NOTE: youre being limited by available memory, there's a better way to structure this 
 -? lots of disk space....

 seperate the trees into four trees
 Solid tree - - transparent tree
 culling tree  for both

Re-do pipeline and object abstraction now that you know what you want
 - Support for multiple types of vertex

Add culling support to the world generator

add basic voxel lighting to the world and ray traced shadows at the granularity of individial voxels
add ambient occlusion


NOAS

replace gcem with cmath once its constexpr