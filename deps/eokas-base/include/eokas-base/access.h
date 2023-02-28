
#ifndef  _EOKAS_BASE_ACCESS_H_
#define  _EOKAS_BASE_ACCESS_H_

#include "header.h"

_BeginNamespace(eokas)
/*
=================================================================
== AccessRef
== A tool which can attach to a value and provide two methods
   to access the value. you can set the read and write params 
   to open or close the functionality of get and set.
=================================================================
*/
template<typename T, bool read=true, bool write=true>
class AccessRef
{
public:
    AccessRef(T& ref)
		:mRef(ref)
	{}

public:
	const T& get() const
	{
		static_assert(read, "object access cannot be read.");
		return mRef;
	}
	AccessRef<T, read, true>& set(const T& ref)
	{
		static_assert(write, "object access cannot be written.");
		mRef = ref;
		return *this;
	}

	operator const T&() const
	{
		return this->get();
	}
    AccessRef<T, read, true>& operator=(const T& ref)
	{
		return this->set(ref);
	}
    AccessRef<T, read, true>& operator=(const AccessRef<T, true, write>& access)
	{
		return this->set(access.mRef);
	}

private:
	T& mRef;
};
/*
=================================================================
== AccessValue
== An object which provide an value storage space and two access
   methods to the value. you can set the read and write params 
   to open or close the functionality of get and set.
=================================================================
*/
template<typename T, bool read = true, bool write = true>
class AccessValue
{
public:
	const T& get() const
	{
		static_assert(read, "access object cannot be read.");
		return mValue;
	}
    AccessValue<T, read, true>& set(const T& value)
	{
		static_assert(write, "access object cannot be written.");
		mValue = value;
		return *this;
	}

	operator const T&() const
	{
		return this->get();
	}
    AccessValue<T, read, true>& operator=(const T& value)
	{
		return this->set(value);
	}
    AccessValue<T, read, true>& operator=(const AccessValue<T, true, write>& object)
	{
		return this->set(object.mValue);
	}

private:
	T mValue;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_ACCESS_H_
