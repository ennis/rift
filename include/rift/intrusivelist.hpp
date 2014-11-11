#ifndef INTRUSIVELIST_HPP
#define INTRUSIVELIST_HPP

//-------------------------------------------------
// Doubly-linked intrusive lists
// (Circularly, actually)
template <typename T>
class ListNode
{
public:
	ListNode() : mNext(0), mPrev(0)
	{}

	void insertAfter(ListNode<T> * node) 
	{
		mNext = node->mNext;
		//if (node->mNext)
		  node->mNext->mPrev = this;
		node->mNext = this;
		mPrev = node;
	}

	void insertBefore(ListNode<T> * node) 
	{
		mPrev = node->mPrev;
		//if (mPrev)
			mPrev->mNext = this;
		node->mPrev = this;
		mNext = node;
	}

	void remove()
	{
		//if (mPrev)
			mPrev->mNext = mNext;
		//if (mNext)
			mNext->mPrev = mPrev;
	}

	T* next()
	{
		return static_cast<T*>(mNext);
	}

	T* prev()
	{
		return static_cast<T*>(mPrev); 
	}

	ListNode<T> * mNext;
	ListNode<T> * mPrev;
};

template <typename T>
class LinkList : public ListNode<T>
{
public:
	LinkList()
	{
		ListNode<T>::mNext = this;
		ListNode<T>::mPrev = this;
	}

	using ListNode<T>::next;
	using ListNode<T>::prev;

	void pushFront(ListNode<T> * node)
	{
		node->insertAfter(this);
	}

	void pushBack(ListNode<T> * node)
	{
		node->insertBefore(this);
	}

	//void push(ListNode<T> * node) { return pushBack(node); }

	T * popFront()
	{
		T * result = first();
		result->remove();
		return result;
	}

	T * popBack()
	{
		T * result = last();
		result->remove();
		return result;
	}

	//T * pop() { return popBack(); }

	bool empty() const {
		return (ListNode<T>::mNext == this);
	}

	T * first()
	{
		return next();
	}

	T * last()
	{
		return prev();
	}

	ListNode<T> * head()
	{
		return this;
	}

	ListNode<T> * tail()
	{
		return this;
	}

	struct Iterator
	{
		Iterator(ListNode<T> *begin) : mCurrent(begin)
		{}

		bool operator!=(Iterator const &right)
		{
			return right.mCurrent != mCurrent;
		}

		void operator++()
		{
			mCurrent = mCurrent->mNext;
		}

		T &operator*() 
		{
			return *static_cast<T*>(mCurrent);
		}

		ListNode<T> *mCurrent;
	};

	Iterator begin() 
	{
		return Iterator(mNext);
	}

	Iterator end() 
	{
		return Iterator(this);
	}

};

#endif