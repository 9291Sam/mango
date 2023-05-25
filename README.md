TODO: custom window icon and cursor

TODO: fix camera to deal with edge cases of tabbing in and out / ESC

TODO: update logger to read an environment variable

TODO: voxel system

TODO: add transparency to vertices


// NOTES:
In order to export a model from blender and have it look right
export with the forward axis as Y
and the Up axis as Z

this preserves the order of the axies
Z and Y are swapped between blender and the renderer
however models facing up are still facing up when imported



TODO octree

normal voxel octree


drawing routine

have a second octree with normal Voxels and a special NODRAW voxel (internal areas, nonvisiable)

iterate over this nodraw tree and all of the voxels that need to be drawn just need face direction checks.

return a list of faces 

greedily mesh this

TODO: fiure out a way to cache this so large parts of the tree can be re-used