#ifndef MESHRENDERABLE_HPP
#define MESHRENDERABLE_HPP

#include <renderable.hpp>
#include <mesh.hpp>

// TODO
class MeshRenderable : public Renderable
{
public:
	MeshRenderable(Renderer &renderer) : Renderable(renderer)
	{}

	~MeshRenderable() {
		if (mMesh) {
			mMesh->release();
		}
	}

	void setMesh(Mesh *mesh);
	//void update(float dt) override;
	void init() override;
	void update(float dt) override;
	void render(RenderContext const &context) override;

private:
	// owning reference
	Mesh *mMesh;
	// TODO multiple materials
	//Material material;
};

#endif