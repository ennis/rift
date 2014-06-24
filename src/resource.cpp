#include <resource.hpp>

void CResourceBase::addRef()
{
	mRefCount++;
}

void CResourceBase::release()
{
	mRefCount--;
}