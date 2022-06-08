
#ifndef  _EOKAS_BASE_DICTIONARY_H_
#define  _EOKAS_BASE_DICTIONARY_H_

#include "header.h"

_BeginNamespace(eokas)

template<typename Index, typename IItem>
class Table
{
	using ItemMap = std::map<Index, IItem*>;

public:
	Table()
		:mItems()
	{}

	~Table()
	{
		this->clear();
	}

	template<typename CItem>
	IItem* insert(const Index& index)
	{
		if(mItems.find(index) != mItems.end())
			return nullptr;
		IItem* item = new CItem();
		mItems[index] = item;
		return item;
	}

	template<typename CItem, typename Arg1>
	IItem* insert(const Index& index, const Arg1& arg1)
	{
		if(mItems.find(index) != mItems.end())
			return nullptr;
		IItem* item = new CItem(arg1);
		mItems[index] = item;
		return item;
	}

	template<typename CItem, typename Arg1, typename Arg2>
	IItem* insert(const Index& index, const Arg1& arg1, const Arg2& arg2)
	{
		if(mItems.find(index) != mItems.end())
			return nullptr;
		IItem* item = new CItem(arg1, arg2);
		mItems[index] = item;
		return item;
	}

	template<typename CItem, typename Arg1, typename Arg2, typename Arg3>
	IItem* insert(const Index& index, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
	{
		if(mItems.find(index) != mItems.end())
			return nullptr;
		IItem* item = new CItem(arg1, arg2, arg3);
		mItems[index] = item;
		return item;
	}

	template<typename CItem, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	IItem* insert(const Index& index, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4)
	{
		if(mItems.find(index) != mItems.end())
			return nullptr;
		IItem* item = new CItem(arg1, arg2, arg3, arg4);
		mItems[index] = item;
		return item;
	}

	template<typename CItem, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
	IItem* insert(const Index& index, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5)
	{
		if(mItems.find(index) != mItems.end())
			return nullptr;
		IItem* item = new CItem(arg1, arg2, arg3, arg4, arg5);
		mItems[index] = item;
		return item;
	}

	IItem* select(const Index& index)
	{
		auto iter = mItems.find(index);
		if(iter == mItems.end())
			return nullptr;
		return iter->second;
	}

	void remove(const Index& index)
	{
		auto iter = mItems.find(index);
		if(iter == mItems.end())
			return;
		IItem* item = iter->second;
		if(item != nullptr)
		{
			delete item;
			item = nullptr;
		}
		mItems.erase(iter);
	}

	void clear()
	{
		_DeleteMap(mItems);
	}
	
private:
	ItemMap mItems;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_DICTIONARY_H_
