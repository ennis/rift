#include <resource.hpp>
#include <log.hpp>
#include <common.hpp>

void Resource::addRef()
{
	mRefCount++;
}

void Resource::release()
{
	mRefCount--;
	assert(mRefCount >= 0);

	if (mRefCount == 0 && mDeletionPolicy == DeletionPolicy::Delete) {
		destroy();
	}
}

void Resource::destroy()
{
	// does nothing
	LOG << "CResourceBase::deleteResource";
}
