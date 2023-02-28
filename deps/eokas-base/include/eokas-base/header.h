
#ifndef  _EOKAS_BASE_HEADER_H_
#define  _EOKAS_BASE_HEADER_H_

/*
=================================================================
== OS
=================================================================
*/
#define _EOKAS_OS_WIN64 1
#define _EOKAS_OS_WIN32 2
#define _EOKAS_OS_LINUX 3
#define _EOKAS_OS_MACOS 4
#define _EOKAS_OS_IOS 5
#define _EOKAS_OS_ANDROID 6

#if defined(_WIN64) || defined(WIN64)
    #define _EOKAS_OS _EOKAS_OS_WIN64
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #define _EOKAS_OS _EOKAS_OS_WIN32
#elif defined(__APPLE_CC__)
    #if (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 30000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000)
        #define _EOKAS_OS _EOKAS_OS_IOS
    #else
        #define _EOKAS_OS _EOKAS_OS_MACOS
    #endif
#elif defined(__ANDROID__)
    #define _EOKAS_OS _EOKAS_OS_ANDROID
#else
    #define _EOKAS_OS _EOKAS_OS_LINUX
#endif

/*
=================================================================
== Compiler-Family and Compiler-Version
=================================================================
*/
#define _EOKAS_COMPILER_FAMILY_MSVC 1
#define _EOKAS_COMPILER_FAMILY_GNUC 2
#define _EOKAS_COMPILER_FAMILY_CLANG 3

#if defined(_MSC_VER)
    #define _EOKAS_COMPILER_FAMILY _EOKAS_COMPILER_FAMILY_MSVC
    #define _EOKAS_COMPILER_VERSION _MSC_VER
#elif defined(__GNUC__)
    #define _EOKAS_COMPILER_FAMILY _EOKAS_COMPILER_GNUC
    #define _EOKAS_COMPILER_VERSION  (((__GNUC__)*100) + ((__GNUC_MINOR__)*10) + (__GNUC_PATCHLEVEL__))
#elif defined(__clang__)
    #define _EOKAS_COMPILER_FAMILY _EOKAS_COMPILER_CLANG
    #define _EOKAS_COMPILER_VERSION __clang_version__
#endif

/*
=================================================================
== CPU ARCH
=================================================================
*/

#define _EOKAS_ARCH_X86 0x00863200
#define _EOKAS_ARCH_X64 0x00866400

#define _EOKAS_ARCH_ARM32   0x000A3200
#define _EOKAS_ARCH_ARM64   0x000A6400

#define _EOKAS_ARCH_MIPS        0x00010000
#define _EOKAS_ARCH_SUPERH      0x00020000
#define _EOKAS_ARCH_POWERPC     0x00033200
#define _EOKAS_ARCH_POWERPC64   0x00036400
#define _EOKAS_ARCH_SPARC       0x00040000
#define _EOKAS_ARCH_M68K        0x00050000

#if defined(_M_X64) || defined(__amd64__) || defined(__x86_64__)
    #define _EOKAS_ARCH _EOKAS_ARCH_X64
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
    #define _EOKAS_ARCH _EOKAS_ARCH_X86
#elif defined(__arm__) || defined(_M_ARM)
    #define _EOKAS_ARCH _EOKAS_ARCH_ARM32
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define _EOKAS_ARCH _EOKAS_ARCH_ARM64
#elif defined(mips) || defined(__mips__) || defined(__mips)
    #define _EOKAS_ARCH _EOKAS_ARCH_MIPS
#elif defined(__sh__)
    #define _EOKAS_ARCH _EOKAS_ARCH_SUPERH
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
    #define _EOKAS_ARCH _EOKAS_ARCH_POWERPC
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
    #define _EOKAS_ARCH _EOKAS_ARCH_POWERPC64
#elif defined(__sparc__) || defined(__sparc)
    #define _EOKAS_ARCH _EOKAS_ARCH_SPARC
#elif defined(__m68k__)
    #define _EOKAS_ARCH _EOKAS_ARCH_M68K
#else
    #define _EOKAS_ARCH _EOKAS_ARCH_UNKNOWN
#endif

/*
=================================================================
== StringType and Unicode
=================================================================
*/
#define _EOKAS_STRINGTYPE_MBS 1
#define _EOKAS_STRINGTYPE_WCS 2

#if (_EOKAS_COMPILER_FAMILY == _EOKAS_COMPILER_FAMILY_MSVC)
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

#if(_EOKAS_OS == _EOKAS_OS_WIN64 || _EOKAS_OS == _EOKAS_OS_WIN32)
    #define _EOKAS_UCS _EOKAS_UCS_2
#elif(_EOKAS_OS == _EOKAS_OS_LINUX)
    #define _EOKAS_UCS _EOKAS_UCS_4
#endif

/*
=================================================================
== Universal macros
=================================================================
*/
#if (_EOKAS_COMPILER_FAMILY == _EOKAS_COMPILER_FAMILY_MSVC)
    #pragma warning(disable: 4996) // do not tell me to use "_s" functions.
#endif

#if (_EOKAS_COMPILER_FAMILY == _EOKAS_COMPILER_FAMILY_MSVC)
    #if (_EOKAS_COMPILER_VERSION < 1600)
        #define nullptr NULL
    #endif
#endif

#if (_EOKAS_COMPILER_FAMILY == _EOKAS_COMPILER_FAMILY_MSVC)
    #ifndef _EOKAS_API_EXPRT
        #define _EOKAS_API __declspec(dllimport)
    #else
        #define _EOKAS_API __declspec(dllexport)
    #endif//_EOKAS_API_EXPRT
#else
    #define _EOKAS_API
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
#include <memory>
#include <functional>

namespace eokas
{
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

        virtual const std::type_info &dataType() {
            return typeid(*this);
        }

        template<typename T>
        bool is() {
            return typeid(T).before(this->dataType());
        }

        template<typename T>
        T* as() {
            return dynamic_cast<T*>(this);
        }
    };
};

#endif//_EOKAS_BASE_HEADER_H_
