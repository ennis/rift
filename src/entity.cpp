#include <entity.hpp>

static LinkList<Entity> sEntityList;

Entity::Entity() 
{
	for (int i = 0; i < kMaxComponents; ++i) {
		mComponents[i] = nullptr;
	}
}

Entity::~Entity()
{
	deleteComponents();
}

void Entity::deleteComponents()
{
	for (int i = 0; i < kMaxComponents; ++i) {
		delete mComponents[i];
	}
}

void Entity::update(float dt)
{
	for (int i = 0; i < kMaxComponents; ++i) {
		if (mComponents[i]) mComponents[i]->update(dt);
	}
}

Entity *Entity::create()
{ 
	// create on the heap (TODO: pool)
	Entity *ent = new Entity;
	// add it to the list
	sEntityList.pushBack(ent);
	return ent;
}

void Entity::destroy(Entity *ent)
{
	// remove node from the linked list
	ent->remove();
	// free memory
	delete ent;
}

LinkList<Entity> &Entity::getList()
{
	return sEntityList;
}