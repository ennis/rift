#include <resource.hpp>
#include <log.hpp>

void CResourceBase::addRef()
{
	mRefCount++;
}

void CResourceBase::release()
{
	mRefCount--;
	assert(mRefCount >= 0);

	if (mRefCount == 0 && mDeletionPolicy == DeletionPolicy::Delete) {
		deleteResource();
	}
}

void CResourceBase::deleteResource()
{
	// does nothing 
	LOG << "CResourceBase::deleteResource";
}