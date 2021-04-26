
#ifndef  _EOKAS_ARCHAISM_HEADER_H_
#define  _EOKAS_ARCHAISM_HEADER_H_

/*
=================================================================
== Platform
=================================================================
*/
#define _EOKAS_PLATFORM_WIN32	1
#define _EOKAS_PLATFORM_LINUX	2
#define _EOKAS_PLATFORM_MACOS	3
#define _EOKAS_PLATFORM_IOS		4
#define _EOKAS_PLATFORM_ANDROID 5

#if defined(_WIN32) || defined(__WIN32__)
#define _EOKAS_PLATFORM _EOKAS_PLATFORM_WIN32
#elif defined(__APPLE_CC__)
#if (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 30000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000)
#define _EOKAS_PLATFORM _EOKAS_PLATFORM_IOS
#else
#define _EOKAS_PLATFORM _EOKAS_PLATFORM_MACOS
#endif
#elif defined(__ANDROID__)
#define _EOKAS_PLATFORM _EOKAS_PLATFORM_ANDROID
#else
#define _EOKAS_PLATFORM _EOKAS_PLATFORM_LINUX
#endif

/*
=================================================================
== Compiler and CompilerVersion
=================================================================
*/
#define _EOKAS_COMPILER_MSVC 1
#define _EOKAS_COMPILER_GNUC 2

#if defined(_MSC_VER)
#define _EOKAS_COMPILER _EOKAS_COMPILER_MSVC
#define _EOKAS_COMPILERVERSION _MSC_VER
#elif defined(__GNUC__)
#define _EOKAS_COMPILER _EOKAS_COMPILER_GNUC
#define _EOKAS_COMPILERVERSION  \
		(((__GNUC__)*100) + ((__GNUC_MINOR__)*10) + (__GNUC_PATCHLEVEL__))
#endif

/*
=================================================================
== StringType and Unicode
=================================================================
*/
#define _EOKAS_STRINGTYPE_MBS 1
#define _EOKAS_STRINGTYPE_WCS 2

#if _EOKAS_COMPILER == _EOKAS_COMPILER_MSVC
#if defined(_UNICODE)
#define _EOKAS_STRINGTYPE _EOKAS_STRINGTYPE_WCS
#else
#define _EOKAS_STRINGTYPE _EOKAS_STRINGTYPE_MBS
#endif
#else
#define _EOKAS_STRINGTYPE _EOKAS_STRINGTYPE_MBS 
#endif

#define _EOKAS_UCS_2 2
#define _EOKAS_UCS_4 4

#if(_EOKAS_PLATFORM == _EOKAS_PLATFORM_WIN32)
#define _EOKAS_UCS _EOKAS_UCS_2
#elif(_EOKAS_PLATFORM == _EOKAS_PLATFORM_LINUX)
#define _EOKAS_UCS _EOKAS_UCS_4
#endif

/*
=================================================================
== Universal macros
=================================================================
*/
#if (_EOKAS_COMPILER == _EOKAS_COMPILER_MSVC)
#pragma warning(disable: 4996) // do not tell me to use "_s" functions.
#endif

#if (_EOKAS_COMPILER == _EOKAS_COMPILER_MSVC)
#if (_EOKAS_COMPILERVERSION < 1600)
#define nullptr NULL
#endif
#endif

#if (_EOKAS_COMPILER == _EOKAS_COMPILER_MSVC)
#ifndef _EOKAS_API_EXPORT
#define _RadixApi __declspec(dllimport)
#else
#define _RadixApi __declspec(dllexport)
#endif//_RadixApi
#else
#define _RadixApi
#endif

/*
=================================================================
== Frequently used types and macros
=================================================================
*/
#ifndef _BeginNamespace
#define _BeginNamespace(ns) namespace ns {
#endif//_BeginNamespace

#ifndef _EndNamespace
#define _EndNamespace(ns) }
#endif//_EndNamespace

#ifndef _TokenToStr
#define _TokenToStr(token) (#token)
#endif//_TokenToStr

#ifndef _CombineToken
#define _CombineToken(token1, token2) (token1##token2)
#endif//_CombineToken

#ifndef _Cast
#define _Cast(type, expr) ((type)(expr))
#endif//_Cast

#ifndef _ArrayLength
#define _ArrayLength(a) (sizeof(a)/sizeof(a[0]))
#endif//_ArrayLength

#ifndef _BitMask
#define _BitMask(n) (1<<(n))
#endif//_BitMask
#ifndef _SetBit
#define _SetBit(value, n) ((value)|_BitMask(n))
#endif//_SetBit
#ifndef _ClearBit
#define _ClearBit(value, n) ((value) & (~(_BitMask(n)))) 
#endif//_ClearBit
#ifndef _ReverseBit
#define _ReverseBit(value, n) ((value)^_BitMask(n))
#endif//_ReverseBit
#ifndef _CheckBit
#define _CheckBit(value, n) ((value)&_BitMask(n))
#endif//_CheckBit

#ifndef _DeletePointer
#define _DeletePointer(ptr) if((ptr) != nullptr){delete (ptr); (ptr) = nullptr;}
#endif//_DeletePointer
#ifndef _DeleteArray
#define _DeleteArray(ptr) if((ptr) != nullptr){delete [](ptr); (ptr) = nullptr;}
#endif//_DeleteArray
#ifndef _DeleteList
#define _DeleteList(list) if(!(list).empty()){auto iter = (list).begin(); while(iter!=(list).end()){_DeletePointer(*iter); ++iter;}(list).clear();}
#endif//_DeleteList
#ifndef _DeleteMap
#define _DeleteMap(map) if(!(map).empty()){auto iter = (map).begin(); while(iter!=(map).end()){_DeletePointer(iter->second); ++iter;}(map).clear();}
#endif//_DeleteMap

#ifndef _TestReturn
#define _TestReturn(expr, value, ret) if((expr)==(value)){return (ret);}
#endif//_TestReturn

#ifndef _ForbidCopy
#define _ForbidCopy(className) className(const className&) = delete
#endif//_ForbidCopy

#ifndef _ForbidMove
#define _ForbidMove(className) className(className&&) = delete
#endif//_ForbidMove

#ifndef _ForbidAssign
#define _ForbidAssign(className) className& operator=(const className&) = delete
#endif//_ForbidAssign

#include <cstdint>
#include <list>
#include <vector>
#include <stack>
#include <queue>
#include <deque>
#include <map>
#include <string>
#include <typeinfo>

_BeginNamespace(eokas)

using byte = unsigned char;

using i8_t = int8_t;
using u8_t = uint8_t;
using i16_t = int16_t;
using u16_t = uint16_t;
using i32_t = int32_t;
using u32_t = uint32_t;
using i64_t = int64_t;
using u64_t = uint64_t;

using f32_t = float;
using f64_t = double;

#ifndef _FloatEqual
#define _FloatEqual(a, b) ((a)>=(b) ? ((a)-(b)<=0.000001f) : ((b)-(a)<=0.000001f))
#endif//_FloatEqual
#ifndef _FloatNotEqual
#define _FloatNotEqual(a, b) ((a)-(b)>0.000001f || (a)-(b)<-0.000001f)
#endif//_FloatNotEqual

using MBString = std::string;
using WCString = std::wstring;
#if(_EOKAS_STRINGTYPE == _EOKAS_STRINGTYPE_MBS)
using StringType = MBString;
#ifndef _T	
#define _T(cstr) (cstr)
#endif//_T
#ifndef _ToStr
#define _ToStr(token) _T(_TokenToStr(token))
#endif//_ToStr
#else
using StringType = WCString;
#ifndef _T	
#define _T(cstr) (_CombineToken(L, cstr))
#endif//_T
#ifndef _ToStr
#define _ToStr(token) _T(_TokenToStr(token))
#endif//_ToStr
#endif//(_EOKAS_STRINGTYPE == _EOKAS_STRINGTYPE_MBS)
using String = class String;

class Interface
{
public:
	virtual ~Interface() {}
};

class Object
{
public:
	virtual ~Object() {}

	virtual const std::type_info& dataType()
	{
		return typeid(*this);
	}

	template<typename T>
	bool is()
	{
		return typeid(T).before(this->dataType());
	}

	template<typename T>
	T as()
	{
		return dynamic_cast<T>(this);
	}
};

_EndNamespace(eokas)

#endif//_EOKAS_ARCHAISM_HEADER_H_
