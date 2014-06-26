#ifndef MODELLOADER_HPP
#define MODELLOADER_HPP

#include <renderer.hpp>

CMeshBuffer *loadMeshFromOBJ(CRenderer &renderer, const char *path);
CModel *loadModelFromOBJ(CRenderer &renderer, const char *path);

#endif