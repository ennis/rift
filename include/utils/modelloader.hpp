#ifndef MODELLOADER_HPP
#define MODELLOADER_HPP

#include <renderer.hpp>

CMeshBufferRef loadMeshFromOBJ(CRenderer &renderer, const char *path);
CModelRef loadModelFromOBJ(CRenderer &renderer, const char *path);

#endif