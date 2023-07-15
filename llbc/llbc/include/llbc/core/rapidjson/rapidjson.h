// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied. See the License for the 
// specific language governing permissions and limitations under the License.

#ifndef LLBC_RAPIDJSON_RAPIDJSON_H_
#define LLBC_RAPIDJSON_RAPIDJSON_H_

/*!\file rapidjson.h
    \brief common definitions and configuration
    
    \see LLBC_RAPIDJSON_CONFIG
 */

/*! \defgroup LLBC_RAPIDJSON_CONFIG RapidJSON configuration
    \brief Configuration macros for library features

    Some RapidJSON features are configurable to adapt the library to a wide
    variety of platforms, environments and usage scenarios.  Most of the
    features can be configured in terms of overridden or predefined
    preprocessor macros at compile-time.

    Some additional customization is available in the \ref LLBC_RAPIDJSON_ERRORS APIs.

    \note These macros should be given on the compiler command-line
          (where applicable)  to avoid inconsistent values when compiling
          different translation units of a single application.
 */

#include <cstdlib>  // malloc(), realloc(), free(), size_t
#include <cstring>  // memset(), memcpy(), memmove(), memcmp()

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_VERSION_STRING
//
// ALWAYS synchronize the following 3 macros with corresponding variables in /CMakeLists.txt.
//

//!@cond LLBC_RAPIDJSON_HIDDEN_FROM_DOXYGEN
// token stringification
#define LLBC_RAPIDJSON_STRINGIFY(x) LLBC_RAPIDJSON_DO_STRINGIFY(x)
#define LLBC_RAPIDJSON_DO_STRINGIFY(x) #x

// token concatenation
#define LLBC_RAPIDJSON_JOIN(X, Y) LLBC_RAPIDJSON_DO_JOIN(X, Y)
#define LLBC_RAPIDJSON_DO_JOIN(X, Y) LLBC_RAPIDJSON_DO_JOIN2(X, Y)
#define LLBC_RAPIDJSON_DO_JOIN2(X, Y) X##Y
//!@endcond

/*! \def LLBC_RAPIDJSON_MAJOR_VERSION
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief Major version of RapidJSON in integer.
*/
/*! \def LLBC_RAPIDJSON_MINOR_VERSION
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief Minor version of RapidJSON in integer.
*/
/*! \def LLBC_RAPIDJSON_PATCH_VERSION
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief Patch version of RapidJSON in integer.
*/
/*! \def LLBC_RAPIDJSON_VERSION_STRING
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief Version of RapidJSON in "<major>.<minor>.<patch>" string format.
*/
#define LLBC_RAPIDJSON_MAJOR_VERSION 1
#define LLBC_RAPIDJSON_MINOR_VERSION 1
#define LLBC_RAPIDJSON_PATCH_VERSION 0
#define LLBC_RAPIDJSON_VERSION_STRING \
    LLBC_RAPIDJSON_STRINGIFY(LLBC_RAPIDJSON_MAJOR_VERSION.LLBC_RAPIDJSON_MINOR_VERSION.LLBC_RAPIDJSON_PATCH_VERSION)

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_NAMESPACE_(BEGIN|END)
/*! \def LLBC_RAPIDJSON_NAMESPACE
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief   provide custom rapidjson namespace

    In order to avoid symbol clashes and/or "One Definition Rule" errors
    between multiple inclusions of (different versions of) RapidJSON in
    a single binary, users can customize the name of the main RapidJSON
    namespace.

    In case of a single nesting level, defining \c LLBC_RAPIDJSON_NAMESPACE
    to a custom name (e.g. \c MyRapidJSON) is sufficient.  If multiple
    levels are needed, both \ref LLBC_RAPIDJSON_NAMESPACE_BEGIN and \ref
    LLBC_RAPIDJSON_NAMESPACE_END need to be defined as well:

    \code
    // in some .cpp file
    #define LLBC_RAPIDJSON_NAMESPACE my::rapidjson
    #define LLBC_RAPIDJSON_NAMESPACE_BEGIN namespace my { namespace rapidjson {
    #define LLBC_RAPIDJSON_NAMESPACE_END   } }
    #include "llbc/core/rapidjson/rapidjson/..."
    \endcode

    \see rapidjson
 */
/*! \def LLBC_RAPIDJSON_NAMESPACE_BEGIN
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief   provide custom rapidjson namespace (opening expression)
    \see LLBC_RAPIDJSON_NAMESPACE
*/
/*! \def LLBC_RAPIDJSON_NAMESPACE_END
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief   provide custom rapidjson namespace (closing expression)
    \see LLBC_RAPIDJSON_NAMESPACE
*/
#ifndef LLBC_RAPIDJSON_NAMESPACE
#define LLBC_RAPIDJSON_NAMESPACE llbc_rapidjson
#endif
#ifndef LLBC_RAPIDJSON_NAMESPACE_BEGIN
#define LLBC_RAPIDJSON_NAMESPACE_BEGIN namespace LLBC_RAPIDJSON_NAMESPACE {
#endif
#ifndef LLBC_RAPIDJSON_NAMESPACE_END
#define LLBC_RAPIDJSON_NAMESPACE_END }
#endif

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_HAS_STDSTRING

#ifndef LLBC_RAPIDJSON_HAS_STDSTRING
#ifdef LLBC_RAPIDJSON_DOXYGEN_RUNNING
#define LLBC_RAPIDJSON_HAS_STDSTRING 1 // force generation of documentation
#else
#define LLBC_RAPIDJSON_HAS_STDSTRING 0 // no std::string support by default
#endif
/*! \def LLBC_RAPIDJSON_HAS_STDSTRING
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief Enable RapidJSON support for \c std::string

    By defining this preprocessor symbol to \c 1, several convenience functions for using
    \ref rapidjson::GenericValue with \c std::string are enabled, especially
    for construction and comparison.

    \hideinitializer
*/
#endif // !defined(LLBC_RAPIDJSON_HAS_STDSTRING)

#if LLBC_RAPIDJSON_HAS_STDSTRING
#include <string>
#endif // LLBC_RAPIDJSON_HAS_STDSTRING

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_NO_INT64DEFINE

/*! \def LLBC_RAPIDJSON_NO_INT64DEFINE
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief Use external 64-bit integer types.

    RapidJSON requires the 64-bit integer types \c int64_t and  \c uint64_t types
    to be available at global scope.

    If users have their own definition, define LLBC_RAPIDJSON_NO_INT64DEFINE to
    prevent RapidJSON from defining its own types.
*/
#ifndef LLBC_RAPIDJSON_NO_INT64DEFINE
//!@cond LLBC_RAPIDJSON_HIDDEN_FROM_DOXYGEN
#if defined(_MSC_VER) && (_MSC_VER < 1800)	// Visual Studio 2013
#include "llbc/core/rapidjson/msinttypes/stdint.h"
#include "llbc/core/rapidjson/msinttypes/inttypes.h"
#else
// Other compilers should have this.
#include <stdint.h>
#include <inttypes.h>
#endif
//!@endcond
#ifdef LLBC_RAPIDJSON_DOXYGEN_RUNNING
#define LLBC_RAPIDJSON_NO_INT64DEFINE
#endif
#endif // LLBC_RAPIDJSON_NO_INT64TYPEDEF

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_FORCEINLINE

#ifndef LLBC_RAPIDJSON_FORCEINLINE
//!@cond LLBC_RAPIDJSON_HIDDEN_FROM_DOXYGEN
#if defined(_MSC_VER) && defined(NDEBUG)
#define LLBC_RAPIDJSON_FORCEINLINE __forceinline
#elif defined(__GNUC__) && __GNUC__ >= 4 && defined(NDEBUG)
#define LLBC_RAPIDJSON_FORCEINLINE __attribute__((always_inline))
#else
#define LLBC_RAPIDJSON_FORCEINLINE
#endif
//!@endcond
#endif // LLBC_RAPIDJSON_FORCEINLINE

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_ENDIAN
#define LLBC_RAPIDJSON_LITTLEENDIAN  0   //!< Little endian machine
#define LLBC_RAPIDJSON_BIGENDIAN     1   //!< Big endian machine

//! Endianness of the machine.
/*!
    \def LLBC_RAPIDJSON_ENDIAN
    \ingroup LLBC_RAPIDJSON_CONFIG

    GCC 4.6 provided macro for detecting endianness of the target machine. But other
    compilers may not have this. User can define LLBC_RAPIDJSON_ENDIAN to either
    \ref LLBC_RAPIDJSON_LITTLEENDIAN or \ref LLBC_RAPIDJSON_BIGENDIAN.

    Default detection implemented with reference to
    \li https://gcc.gnu.org/onlinedocs/gcc-4.6.0/cpp/Common-Predefined-Macros.html
    \li http://www.boost.org/doc/libs/1_42_0/boost/detail/endian.hpp
*/
#ifndef LLBC_RAPIDJSON_ENDIAN
// Detect with GCC 4.6's macro
#  ifdef __BYTE_ORDER__
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#      define LLBC_RAPIDJSON_ENDIAN LLBC_RAPIDJSON_LITTLEENDIAN
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#      define LLBC_RAPIDJSON_ENDIAN LLBC_RAPIDJSON_BIGENDIAN
#    else
#      error Unknown machine endianness detected. User needs to define LLBC_RAPIDJSON_ENDIAN.
#    endif // __BYTE_ORDER__
// Detect with GLIBC's endian.h
#  elif defined(__GLIBC__)
#    include <endian.h>
#    if (__BYTE_ORDER == __LITTLE_ENDIAN)
#      define LLBC_RAPIDJSON_ENDIAN LLBC_RAPIDJSON_LITTLEENDIAN
#    elif (__BYTE_ORDER == __BIG_ENDIAN)
#      define LLBC_RAPIDJSON_ENDIAN LLBC_RAPIDJSON_BIGENDIAN
#    else
#      error Unknown machine endianness detected. User needs to define LLBC_RAPIDJSON_ENDIAN.
#   endif // __GLIBC__
// Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro
#  elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#    define LLBC_RAPIDJSON_ENDIAN LLBC_RAPIDJSON_LITTLEENDIAN
#  elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#    define LLBC_RAPIDJSON_ENDIAN LLBC_RAPIDJSON_BIGENDIAN
// Detect with architecture macros
#  elif defined(__sparc) || defined(__sparc__) || defined(_POWER) || defined(__powerpc__) || defined(__ppc__) || defined(__hpux) || defined(__hppa) || defined(_MIPSEB) || defined(_POWER) || defined(__s390__)
#    define LLBC_RAPIDJSON_ENDIAN LLBC_RAPIDJSON_BIGENDIAN
#  elif defined(__i386__) || defined(__alpha__) || defined(__ia64) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__bfin__)
#    define LLBC_RAPIDJSON_ENDIAN LLBC_RAPIDJSON_LITTLEENDIAN
#  elif defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
#    define LLBC_RAPIDJSON_ENDIAN LLBC_RAPIDJSON_LITTLEENDIAN
#  elif defined(LLBC_RAPIDJSON_DOXYGEN_RUNNING)
#    define LLBC_RAPIDJSON_ENDIAN
#  else
#    error Unknown machine endianness detected. User needs to define LLBC_RAPIDJSON_ENDIAN.   
#  endif
#endif // LLBC_RAPIDJSON_ENDIAN

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_64BIT

//! Whether using 64-bit architecture
#ifndef LLBC_RAPIDJSON_64BIT
#if defined(__LP64__) || (defined(__x86_64__) && defined(__ILP32__)) || defined(_WIN64) || defined(__EMSCRIPTEN__)
#define LLBC_RAPIDJSON_64BIT 1
#else
#define LLBC_RAPIDJSON_64BIT 0
#endif
#endif // LLBC_RAPIDJSON_64BIT

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_ALIGN

//! Data alignment of the machine.
/*! \ingroup LLBC_RAPIDJSON_CONFIG
    \param x pointer to align

    Some machines require strict data alignment. The default is 8 bytes.
    User can customize by defining the LLBC_RAPIDJSON_ALIGN function macro.
*/
#ifndef LLBC_RAPIDJSON_ALIGN
#define LLBC_RAPIDJSON_ALIGN(x) (((x) + static_cast<size_t>(7u)) & ~static_cast<size_t>(7u))
#endif

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_UINT64_C2

//! Construct a 64-bit literal by a pair of 32-bit integer.
/*!
    64-bit literal with or without ULL suffix is prone to compiler warnings.
    UINT64_C() is C macro which cause compilation problems.
    Use this macro to define 64-bit constants by a pair of 32-bit integer.
*/
#ifndef LLBC_RAPIDJSON_UINT64_C2
#define LLBC_RAPIDJSON_UINT64_C2(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))
#endif

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_48BITPOINTER_OPTIMIZATION

//! Use only lower 48-bit address for some pointers.
/*!
    \ingroup LLBC_RAPIDJSON_CONFIG

    This optimization uses the fact that current X86-64 architecture only implement lower 48-bit virtual address.
    The higher 16-bit can be used for storing other data.
    \c GenericValue uses this optimization to reduce its size form 24 bytes to 16 bytes in 64-bit architecture.
*/
#ifndef LLBC_RAPIDJSON_48BITPOINTER_OPTIMIZATION
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define LLBC_RAPIDJSON_48BITPOINTER_OPTIMIZATION 1
#else
#define LLBC_RAPIDJSON_48BITPOINTER_OPTIMIZATION 0
#endif
#endif // LLBC_RAPIDJSON_48BITPOINTER_OPTIMIZATION

#if LLBC_RAPIDJSON_48BITPOINTER_OPTIMIZATION == 1
#if LLBC_RAPIDJSON_64BIT != 1
#error LLBC_RAPIDJSON_48BITPOINTER_OPTIMIZATION can only be set to 1 when LLBC_RAPIDJSON_64BIT=1
#endif
#define LLBC_RAPIDJSON_SETPOINTER(type, p, x) (p = reinterpret_cast<type *>((reinterpret_cast<uintptr_t>(p) & static_cast<uintptr_t>(LLBC_RAPIDJSON_UINT64_C2(0xFFFF0000, 0x00000000))) | reinterpret_cast<uintptr_t>(reinterpret_cast<const void*>(x))))
#define LLBC_RAPIDJSON_GETPOINTER(type, p) (reinterpret_cast<type *>(reinterpret_cast<uintptr_t>(p) & static_cast<uintptr_t>(LLBC_RAPIDJSON_UINT64_C2(0x0000FFFF, 0xFFFFFFFF))))
#else
#define LLBC_RAPIDJSON_SETPOINTER(type, p, x) (p = (x))
#define LLBC_RAPIDJSON_GETPOINTER(type, p) (p)
#endif

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_SSE2/LLBC_RAPIDJSON_SSE42/LLBC_RAPIDJSON_NEON/LLBC_RAPIDJSON_SIMD

/*! \def LLBC_RAPIDJSON_SIMD
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief Enable SSE2/SSE4.2/Neon optimization.

    RapidJSON supports optimized implementations for some parsing operations
    based on the SSE2, SSE4.2 or NEon SIMD extensions on modern Intel
    or ARM compatible processors.

    To enable these optimizations, three different symbols can be defined;
    \code
    // Enable SSE2 optimization.
    #define LLBC_RAPIDJSON_SSE2

    // Enable SSE4.2 optimization.
    #define LLBC_RAPIDJSON_SSE42
    \endcode

    // Enable ARM Neon optimization.
    #define LLBC_RAPIDJSON_NEON
    \endcode

    \c LLBC_RAPIDJSON_SSE42 takes precedence over SSE2, if both are defined.

    If any of these symbols is defined, RapidJSON defines the macro
    \c LLBC_RAPIDJSON_SIMD to indicate the availability of the optimized code.
*/
#if defined(LLBC_RAPIDJSON_SSE2) || defined(LLBC_RAPIDJSON_SSE42) \
    || defined(LLBC_RAPIDJSON_NEON) || defined(LLBC_RAPIDJSON_DOXYGEN_RUNNING)
#define LLBC_RAPIDJSON_SIMD
#endif

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_NO_SIZETYPEDEFINE

#define LLBC_RAPIDJSON_NO_SIZETYPEDEFINE
#ifndef LLBC_RAPIDJSON_NO_SIZETYPEDEFINE
/*! \def LLBC_RAPIDJSON_NO_SIZETYPEDEFINE
    \ingroup LLBC_RAPIDJSON_CONFIG
    \brief User-provided \c SizeType definition.

    In order to avoid using 32-bit size types for indexing strings and arrays,
    define this preprocessor symbol and provide the type rapidjson::SizeType
    before including RapidJSON:
    \code
    #define LLBC_RAPIDJSON_NO_SIZETYPEDEFINE
    namespace rapidjson { typedef ::std::size_t SizeType; }
    #include "llbc/core/rapidjson/rapidjson/..."
    \endcode

    \see rapidjson::SizeType
*/
#ifdef LLBC_RAPIDJSON_DOXYGEN_RUNNING
#define LLBC_RAPIDJSON_NO_SIZETYPEDEFINE
#endif
LLBC_RAPIDJSON_NAMESPACE_BEGIN
//! Size type (for string lengths, array sizes, etc.)
/*! RapidJSON uses 32-bit array/string indices even on 64-bit platforms,
    instead of using \c size_t. Users may override the SizeType by defining
    \ref LLBC_RAPIDJSON_NO_SIZETYPEDEFINE.
*/
typedef unsigned SizeType;
LLBC_RAPIDJSON_NAMESPACE_END
#else
typedef std::size_t SizeType;
#endif

// always import std::size_t to rapidjson namespace
LLBC_RAPIDJSON_NAMESPACE_BEGIN
using std::size_t;
LLBC_RAPIDJSON_NAMESPACE_END

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_ASSERT

//! Assertion.
/*! \ingroup LLBC_RAPIDJSON_CONFIG
    By default, rapidjson uses C \c assert() for internal assertions.
    User can override it by defining LLBC_RAPIDJSON_ASSERT(x) macro.

    \note Parsing errors are handled and can be customized by the
          \ref LLBC_RAPIDJSON_ERRORS APIs.
*/
#ifndef LLBC_RAPIDJSON_ASSERT
#include <cassert>
#define LLBC_RAPIDJSON_ASSERT(x) assert(x)
#endif // LLBC_RAPIDJSON_ASSERT

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_STATIC_ASSERT

// Prefer C++11 static_assert, if available
#ifndef LLBC_RAPIDJSON_STATIC_ASSERT
#if __cplusplus >= 201103L || ( defined(_MSC_VER) && _MSC_VER >= 1800 )
#define LLBC_RAPIDJSON_STATIC_ASSERT(x) \
   static_assert(x, LLBC_RAPIDJSON_STRINGIFY(x))
#endif // C++11
#endif // LLBC_RAPIDJSON_STATIC_ASSERT

// Adopt C++03 implementation from boost
#ifndef LLBC_RAPIDJSON_STATIC_ASSERT
#ifndef __clang__
//!@cond LLBC_RAPIDJSON_HIDDEN_FROM_DOXYGEN
#endif
LLBC_RAPIDJSON_NAMESPACE_BEGIN
template <bool x> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
template <size_t x> struct StaticAssertTest {};
LLBC_RAPIDJSON_NAMESPACE_END

#if defined(__GNUC__) || defined(__clang__)
#define LLBC_RAPIDJSON_STATIC_ASSERT_UNUSED_ATTRIBUTE __attribute__((unused))
#else
#define LLBC_RAPIDJSON_STATIC_ASSERT_UNUSED_ATTRIBUTE 
#endif
#ifndef __clang__
//!@endcond
#endif

/*! \def LLBC_RAPIDJSON_STATIC_ASSERT
    \brief (Internal) macro to check for conditions at compile-time
    \param x compile-time condition
    \hideinitializer
 */
#define LLBC_RAPIDJSON_STATIC_ASSERT(x) \
    typedef ::LLBC_RAPIDJSON_NAMESPACE::StaticAssertTest< \
      sizeof(::LLBC_RAPIDJSON_NAMESPACE::STATIC_ASSERTION_FAILURE<bool(x) >)> \
    LLBC_RAPIDJSON_JOIN(StaticAssertTypedef, __LINE__) LLBC_RAPIDJSON_STATIC_ASSERT_UNUSED_ATTRIBUTE
#endif // LLBC_RAPIDJSON_STATIC_ASSERT

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_LIKELY, LLBC_RAPIDJSON_UNLIKELY

//! Compiler branching hint for expression with high probability to be true.
/*!
    \ingroup LLBC_RAPIDJSON_CONFIG
    \param x Boolean expression likely to be true.
*/
#ifndef LLBC_RAPIDJSON_LIKELY
#if defined(__GNUC__) || defined(__clang__)
#define LLBC_RAPIDJSON_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define LLBC_RAPIDJSON_LIKELY(x) (x)
#endif
#endif

//! Compiler branching hint for expression with low probability to be true.
/*!
    \ingroup LLBC_RAPIDJSON_CONFIG
    \param x Boolean expression unlikely to be true.
*/
#ifndef LLBC_RAPIDJSON_UNLIKELY
#if defined(__GNUC__) || defined(__clang__)
#define LLBC_RAPIDJSON_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LLBC_RAPIDJSON_UNLIKELY(x) (x)
#endif
#endif

///////////////////////////////////////////////////////////////////////////////
// Helpers

//!@cond LLBC_RAPIDJSON_HIDDEN_FROM_DOXYGEN

#define LLBC_RAPIDJSON_MULTILINEMACRO_BEGIN do {  
#define LLBC_RAPIDJSON_MULTILINEMACRO_END \
} while((void)0, 0)

// adopted from Boost
#define LLBC_RAPIDJSON_VERSION_CODE(x,y,z) \
  (((x)*100000) + ((y)*100) + (z))

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_DIAG_PUSH/POP, LLBC_RAPIDJSON_DIAG_OFF

#if defined(__GNUC__)
#define LLBC_RAPIDJSON_GNUC \
    LLBC_RAPIDJSON_VERSION_CODE(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__)
#endif

#if defined(__clang__) || (defined(LLBC_RAPIDJSON_GNUC) && LLBC_RAPIDJSON_GNUC >= LLBC_RAPIDJSON_VERSION_CODE(4,2,0))

#define LLBC_RAPIDJSON_PRAGMA(x) _Pragma(LLBC_RAPIDJSON_STRINGIFY(x))
#define LLBC_RAPIDJSON_DIAG_PRAGMA(x) LLBC_RAPIDJSON_PRAGMA(GCC diagnostic x)
#define LLBC_RAPIDJSON_DIAG_OFF(x) \
    LLBC_RAPIDJSON_DIAG_PRAGMA(ignored LLBC_RAPIDJSON_STRINGIFY(LLBC_RAPIDJSON_JOIN(-W,x)))

// push/pop support in Clang and GCC>=4.6
#if defined(__clang__) || (defined(LLBC_RAPIDJSON_GNUC) && LLBC_RAPIDJSON_GNUC >= LLBC_RAPIDJSON_VERSION_CODE(4,6,0))
#define LLBC_RAPIDJSON_DIAG_PUSH LLBC_RAPIDJSON_DIAG_PRAGMA(push)
#define LLBC_RAPIDJSON_DIAG_POP  LLBC_RAPIDJSON_DIAG_PRAGMA(pop)
#else // GCC >= 4.2, < 4.6
#define LLBC_RAPIDJSON_DIAG_PUSH /* ignored */
#define LLBC_RAPIDJSON_DIAG_POP /* ignored */
#endif

#elif defined(_MSC_VER)

// pragma (MSVC specific)
#define LLBC_RAPIDJSON_PRAGMA(x) __pragma(x)
#define LLBC_RAPIDJSON_DIAG_PRAGMA(x) LLBC_RAPIDJSON_PRAGMA(warning(x))

#define LLBC_RAPIDJSON_DIAG_OFF(x) LLBC_RAPIDJSON_DIAG_PRAGMA(disable: x)
#define LLBC_RAPIDJSON_DIAG_PUSH LLBC_RAPIDJSON_DIAG_PRAGMA(push)
#define LLBC_RAPIDJSON_DIAG_POP  LLBC_RAPIDJSON_DIAG_PRAGMA(pop)

#else

#define LLBC_RAPIDJSON_DIAG_OFF(x) /* ignored */
#define LLBC_RAPIDJSON_DIAG_PUSH   /* ignored */
#define LLBC_RAPIDJSON_DIAG_POP    /* ignored */

#endif // LLBC_RAPIDJSON_DIAG_*

///////////////////////////////////////////////////////////////////////////////
// C++11 features

#ifndef LLBC_RAPIDJSON_HAS_CXX11_RVALUE_REFS
#if defined(__clang__)
#if __has_feature(cxx_rvalue_references) && \
    (defined(_MSC_VER) || defined(_LIBCPP_VERSION) || defined(__GLIBCXX__) && __GLIBCXX__ >= 20080306)
#define LLBC_RAPIDJSON_HAS_CXX11_RVALUE_REFS 1
#else
#define LLBC_RAPIDJSON_HAS_CXX11_RVALUE_REFS 0
#endif
#elif (defined(LLBC_RAPIDJSON_GNUC) && (LLBC_RAPIDJSON_GNUC >= LLBC_RAPIDJSON_VERSION_CODE(4,3,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
      (defined(_MSC_VER) && _MSC_VER >= 1600) || \
      (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5140 && defined(__GXX_EXPERIMENTAL_CXX0X__))

#define LLBC_RAPIDJSON_HAS_CXX11_RVALUE_REFS 1
#else
#define LLBC_RAPIDJSON_HAS_CXX11_RVALUE_REFS 0
#endif
#endif // LLBC_RAPIDJSON_HAS_CXX11_RVALUE_REFS

#ifndef LLBC_RAPIDJSON_HAS_CXX11_NOEXCEPT
#if defined(__clang__)
#define LLBC_RAPIDJSON_HAS_CXX11_NOEXCEPT __has_feature(cxx_noexcept)
#elif (defined(LLBC_RAPIDJSON_GNUC) && (LLBC_RAPIDJSON_GNUC >= LLBC_RAPIDJSON_VERSION_CODE(4,6,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
    (defined(_MSC_VER) && _MSC_VER >= 1900) || \
    (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5140 && defined(__GXX_EXPERIMENTAL_CXX0X__))
#define LLBC_RAPIDJSON_HAS_CXX11_NOEXCEPT 1
#else
#define LLBC_RAPIDJSON_HAS_CXX11_NOEXCEPT 0
#endif
#endif
#if LLBC_RAPIDJSON_HAS_CXX11_NOEXCEPT
#define LLBC_RAPIDJSON_NOEXCEPT noexcept
#else
#define LLBC_RAPIDJSON_NOEXCEPT /* noexcept */
#endif // LLBC_RAPIDJSON_HAS_CXX11_NOEXCEPT

// no automatic detection, yet
#ifndef LLBC_RAPIDJSON_HAS_CXX11_TYPETRAITS
#if (defined(_MSC_VER) && _MSC_VER >= 1700)
#define LLBC_RAPIDJSON_HAS_CXX11_TYPETRAITS 1
#else
#define LLBC_RAPIDJSON_HAS_CXX11_TYPETRAITS 0
#endif
#endif

#ifndef LLBC_RAPIDJSON_HAS_CXX11_RANGE_FOR
#if defined(__clang__)
#define LLBC_RAPIDJSON_HAS_CXX11_RANGE_FOR __has_feature(cxx_range_for)
#elif (defined(LLBC_RAPIDJSON_GNUC) && (LLBC_RAPIDJSON_GNUC >= LLBC_RAPIDJSON_VERSION_CODE(4,6,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
      (defined(_MSC_VER) && _MSC_VER >= 1700) || \
      (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5140 && defined(__GXX_EXPERIMENTAL_CXX0X__))
#define LLBC_RAPIDJSON_HAS_CXX11_RANGE_FOR 1
#else
#define LLBC_RAPIDJSON_HAS_CXX11_RANGE_FOR 0
#endif
#endif // LLBC_RAPIDJSON_HAS_CXX11_RANGE_FOR

//!@endcond

//! Assertion (in non-throwing contexts).
 /*! \ingroup LLBC_RAPIDJSON_CONFIG
    Some functions provide a \c noexcept guarantee, if the compiler supports it.
    In these cases, the \ref LLBC_RAPIDJSON_ASSERT macro cannot be overridden to
    throw an exception.  This macro adds a separate customization point for
    such cases.

    Defaults to C \c assert() (as \ref LLBC_RAPIDJSON_ASSERT), if \c noexcept is
    supported, and to \ref LLBC_RAPIDJSON_ASSERT otherwise.
 */

///////////////////////////////////////////////////////////////////////////////
// LLBC_RAPIDJSON_NOEXCEPT_ASSERT

#ifndef LLBC_RAPIDJSON_NOEXCEPT_ASSERT
#ifdef LLBC_RAPIDJSON_ASSERT_THROWS
#if LLBC_RAPIDJSON_HAS_CXX11_NOEXCEPT
#define LLBC_RAPIDJSON_NOEXCEPT_ASSERT(x)
#else
#define LLBC_RAPIDJSON_NOEXCEPT_ASSERT(x) LLBC_RAPIDJSON_ASSERT(x)
#endif // LLBC_RAPIDJSON_HAS_CXX11_NOEXCEPT
#else
#define LLBC_RAPIDJSON_NOEXCEPT_ASSERT(x) LLBC_RAPIDJSON_ASSERT(x)
#endif // LLBC_RAPIDJSON_ASSERT_THROWS
#endif // LLBC_RAPIDJSON_NOEXCEPT_ASSERT

///////////////////////////////////////////////////////////////////////////////
// new/delete

#ifndef LLBC_RAPIDJSON_NEW
///! customization point for global \c new
#define LLBC_RAPIDJSON_NEW(TypeName) new TypeName
#endif
#ifndef LLBC_RAPIDJSON_DELETE
///! customization point for global \c delete
#define LLBC_RAPIDJSON_DELETE(x) delete x
#endif

///////////////////////////////////////////////////////////////////////////////
// Type

/*! \namespace rapidjson
    \brief main RapidJSON namespace
    \see LLBC_RAPIDJSON_NAMESPACE
*/
LLBC_RAPIDJSON_NAMESPACE_BEGIN

//! Type of JSON value
enum Type {
    kNullType = 0,      //!< null
    kFalseType = 1,     //!< false
    kTrueType = 2,      //!< true
    kObjectType = 3,    //!< object
    kArrayType = 4,     //!< array 
    kStringType = 5,    //!< string
    kNumberType = 6     //!< number
};

LLBC_RAPIDJSON_NAMESPACE_END

#endif // LLBC_RAPIDJSON_RAPIDJSON_H_
