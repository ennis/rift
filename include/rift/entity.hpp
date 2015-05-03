#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include <transform.hpp>
#include <intrusivelist.hpp>
#include <functional>

class Entity;

static const unsigned int kMaxComponents = 64;

// If you need to add a new component ID, add it here
enum ComponentIDs
{
	CID_Transform,
	CID_Renderable,
	CID_Camera,
	CID_CameraController
};

// TODO about components:
// A fixed set of component interfaces (RenderElement, Camera, CameraController)
// with multiple implementations:
// RenderElements: ...
// Camera: OrthoCamera, PerspectiveCamera
// CameraController: FreeCameraController, PlayerCameraController
// Health: DefaultEnemyHealth, BossHealth, PlayerHealth
// Script/AI: ...
// 
// No component base class! No component IDs!
// get/set for each component

class Component
{
public:
	Component() = default;

	virtual ~Component()
	{}

	void setEntity(Entity *entity)
	{
		mEntity = entity;
	}

	Entity *getEntity() const {
		return mEntity;
	}

	// update method
	virtual void init() = 0;
	virtual void update(float dt) = 0;

protected:
	// attached entity
	Entity *mEntity = nullptr;
};

template <uint32_t ComponentID>
class IComponent : public Component
{
public:
	static const uint32_t kComponentID = ComponentID;
};

class Entity : public ListNode<Entity>
{
public:
	template <typename T>
	T *getComponent()
	{
		return static_cast<T*>(mComponents[T::kComponentID]);
	}

	template <typename T>
	T *addComponent()
	{
		T *component = new T();
		component->setEntity(this);
		mComponents[T::kComponentID] = component;
		component->init();
		return component;
	}

	template <typename T, typename... Args>
	T *addComponent(Args&&... args)
	{
		T *component = new T(std::forward<Args>(args)...);
		component->setEntity(this);
		mComponents[T::kComponentID] = component;
		component->init();
		return component;
	}

	template <typename T>
	void removeComponent()
	{
		T *compPtr = getComponent<T>();
		delete compPtr; 
		mComponents[T::kComponentID] = nullptr;
	}
	
	template <typename T>
	bool hasComponent()
	{
		return mComponents[T::kComponentID] != nullptr;
	}


	Transform &getTransform() {
		return mTransform;
	}

	void update(float dt);

	static Entity *create();
	static void destroy(Entity *entity);

	Entity();
	~Entity();
	void deleteComponents();

	static LinkList<Entity> &getList(); 

protected:
	// unused?
	uint64_t mId;
	// local-space transform
	Transform mTransform;
	// component table
	// unique_ptrs (Entities own their components)
	Component *mComponents[kMaxComponents];
	// TODO: Name, flags, etc.
	// Render elements?
};

#endif