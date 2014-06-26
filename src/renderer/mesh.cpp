#include <mesh.hpp>
#include <renderer.hpp>

void CMesh::destroy()
{
	LOG << "CMesh::destroy";
	delete this;
}
