
#ifndef  _EOKAS_BASE_POOL_H_
#define  _EOKAS_BASE_POOL_H_

#include "header.h"
#include <functional>

_BeginNamespace(eokas)

template<typename TObject>
class Pool
{
public:
	using Container = std::list<TObject*>;
	using Iterator = typename Container::iterator;

public:
	Pool() :mAlive(), mDeath() { }
	~Pool() { this->destroy(); }

public:
	bool empty() const 
	{ 
		return mAlive.empty();
	}
	size_t size() const 
	{ 
		return mAlive.size(); 
	}
	Iterator begin()
	{ 
		return mAlive.begin(); 
	}
	Iterator end()
	{ 
		return mAlive.end(); 
	}

	template<typename... Args>
	TObject* acquire(Args... args)
	{
		TObject* ptr = nullptr;
		if(!mDeath.empty())
		{
			ptr = mDeath.front();
			mDeath.pop_front();
			new(ptr)TObject(args...);
		}
		else
		{
			ptr = new TObject(args...);
			mAlive.push_back(ptr);
		}
		return ptr;
	}

	void release(TObject* o)
	{
		if(o == nullptr)
			return;		
		mDeath.push_back(o);
		mAlive.remove(o);
	}

	void releaseFront()
	{
		TObject* o = mAlive.front();
		if (o != nullptr)
		{
			mDeath.push_back(0);
		}
		mAlive.pop_front();
	}

	void releaseBack()
	{
		TObject* o = mAlive.back();
		if (o != nullptr)
		{
			mDeath.push_back(0);
		}
		mAlive.pop_back();
	}

	void releaseAll(std::function<bool(TObject*)> predicate)
	{
		if (mAlive.empty())
			return;
		auto iter = mAlive.begin();
		while (iter != mAlive.end())
		{
			TObject* o = *iter;
			if (o != nullptr && predicate(o))
			{
				mDeath.push_back(o);
			}
			++iter;
		}
		mAlive.clear();
	}

	void releaseAll()
	{
		if(mAlive.empty())
			return;
		auto iter = mAlive.begin();
		while(iter != mAlive.end())
		{
			TObject* o = *iter;
			mDeath.push_back(o);
			++ iter;
		}
		mAlive.clear();
	}

	void destroy()
	{
		this->releaseAll();
		if(mDeath.empty())
			return;
		auto iter = mDeath.begin();
		while(iter != mDeath.end())
		{
			TObject* o = *iter;
			_DeletePointer(o);
			++ iter;
		}
		mDeath.clear();
	}

private:
	std::list<TObject*> mAlive;
	std::list<TObject*> mDeath;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_ASCIL_H_
