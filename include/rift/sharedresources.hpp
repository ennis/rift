#ifndef SHAREDRESOURCES_HPP
#define SHAREDRESOURCES_HPP

#include <renderer.hpp>

// TODO RAII

// Common vertex layouts
// Position only, 3D 
extern VertexLayout *GVertexLayout_V3F;
// Position only, 2D
extern VertexLayout *GVertexLayout_V2F;

// Vertex buffers
// Full screen quad ([-1;1]), V2F, TriangleStrip
// TODO do not use a buffer
extern VertexBuffer *GScreenQuad;

// create the shared resources
void initSharedResources(Renderer &renderer);
void releaseSharedResources();


#endif