#pragma once 

// Clang attributes
// https://clang.llvm.org/docs/AttributeReference.html#always-inline-force-inline
// Clang builtins
// https://clang.llvm.org/docs/LanguageExtensions.html

// Msvc attributes
// https://learn.microsoft.com/en-us/cpp/cpp/declspec?view=msvc-170

// Gcc attributes
// https://ohse.de/uwe/articles/gcc-attributes.html and https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/AlignmentMacros.h>
#include <raze/compatibility/BranchPrediction.h>
#include <raze/compatibility/CallingConventions.h>
#include <raze/compatibility/Inline.h>
#include <raze/compatibility/CxxVersionDetection.h>
#include <raze/compatibility/FunctionAttributes.h>
#include <raze/compatibility/LanguageFeatures.h>
#include <raze/compatibility/MemoryMacros.h>
#include <raze/compatibility/Nodiscard.h>
#include <raze/compatibility/SimdCompatibility.h>
#include <raze/compatibility/SystemDetection.h>
#include <raze/compatibility/UnreachableCode.h>
#include <raze/compatibility/Warnings.h>
#include <raze/compatibility/StaticOperators.h>
#include <raze/compatibility/StackProtection.h>
#include <raze/compatibility/Unroll.h>
#include <raze/compatibility/TargetClones.h>

#include <cstddef>

raze_disable_warning_msvc(4067)

__RAZE_COMPATIBILITY_NAMESPACE_BEGIN

constexpr inline bool is_cxx11 = (raze_has_cxx11 == 1);
constexpr inline bool is_cxx14 = (raze_has_cxx14 == 1);
constexpr inline bool is_cxx17 = (raze_has_cxx17 == 1);
constexpr inline bool is_cxx20 = (raze_has_cxx20 == 1);
constexpr inline bool is_cxx23 = (raze_has_cxx23 == 1);

#if defined(raze_os_apple)
    constexpr inline bool is_apple = true;
#else
    constexpr inline bool is_apple = false;
#endif

#if defined(raze_os_darwin)
    constexpr inline bool is_darwin = true;
#else
    constexpr inline bool is_darwin = false;
#endif

#if defined(raze_os_bsd4)
    constexpr inline bool is_bsd4 = true;
#else
    constexpr inline bool is_bsd4 = false;
#endif

#if defined(raze_os_mac)
    constexpr inline bool is_mac = true;
#else
    constexpr inline bool is_mac = false;
#endif

#if defined(raze_os_cygwin)
    constexpr inline bool is_cygwin = true;
#else
    constexpr inline bool is_cygwin = false;
#endif

#if defined(raze_os_win32)
    constexpr inline bool is_win32 = true;
#else
    constexpr inline bool is_win32 = false;
#endif

#if defined(raze_os_win64)
    constexpr inline bool is_win64 = true;
#else
    constexpr inline bool is_win64 = false;
#endif

#if defined(raze_os_linux)
    constexpr inline bool is_linux = true;
#else
    constexpr inline bool is_linux = false;
#endif

#if defined(raze_os_lynx)
    constexpr inline bool is_lynx = true;
#else
    constexpr inline bool is_lynx = false;
#endif

#if defined(raze_os_hurd)
    constexpr inline bool is_hurd = true;
#else
    constexpr inline bool is_hurd = false;
#endif

#if defined(raze_os_freebsd)
    constexpr inline bool is_freebsd = true;
#else
    constexpr inline bool is_freebsd = false;
#endif

#if defined(raze_os_netbsd)
    constexpr inline bool is_netbsd = true;
#else
    constexpr inline bool is_netbsd = false;
#endif

#if defined(raze_os_openbsd)
    constexpr inline bool is_openbsd = true;
#else
    constexpr inline bool is_openbsd = false;
#endif

#if defined(raze_os_dragonfly)
    constexpr inline bool is_dragonfly = true;
#else
    constexpr inline bool is_dragonfly = false;
#endif

#if defined(raze_os_nacl)
    constexpr inline bool is_nacl = true;
#else
    constexpr inline bool is_nacl = false;
#endif

#if defined(raze_os_emscripten)
    constexpr inline bool is_emscripten = true;
#else
    constexpr inline bool is_emscripten = false;
#endif

#if defined(raze_os_rtems)
    constexpr inline bool is_rtems = true;
#else
    constexpr inline bool is_rtems = false;
#endif

#if defined(raze_os_fuchsia)
    constexpr inline bool is_fuchsia = true;
#else
    constexpr inline bool is_fuchsia = false;
#endif

#if defined(raze_os_solaris)
    constexpr inline bool is_solaris = true;
#else
    constexpr inline bool is_solaris = false;
#endif

#if defined(raze_os_qnx)
    constexpr inline bool is_qnx = true;
#else
    constexpr inline bool is_qnx = false;
#endif

#if defined(raze_os_zos)
    constexpr inline bool is_zos = true;
#else
    constexpr inline bool is_zos = false;
#endif

#if defined(raze_os_qurt)
    constexpr inline bool is_qurt = true;
#else
    constexpr inline bool is_qurt = false;
#endif

#if defined(raze_os_windows)
    constexpr inline bool is_windows = true;
#else
    constexpr inline bool is_windows = false;
#endif

#if defined(raze_os_win)
    constexpr inline bool is_win = true;
#else
    constexpr inline bool is_win = false;
#endif

#if defined(raze_os_unix)
    constexpr inline bool is_unix = true;
#else
    constexpr inline bool is_unix = false;
#endif

#if defined(raze_cpp_icpx)
    constexpr inline bool is_icpx = true;
    constexpr inline int  icpx_version = raze_cpp_icpx;
#else
    constexpr inline bool is_icpx = false;
    constexpr inline int  icpx_version = 0;
#endif

#if defined(raze_cpp_icc)
    constexpr inline bool is_icc = true;
    constexpr inline int  icc_version = raze_cpp_icc;
#else
    constexpr inline bool is_icc = false;
    constexpr inline int  icc_version = 0;
#endif

#if defined(raze_cpp_msvc)
    constexpr inline bool is_msvc = true;
    constexpr inline int  msvc_version = raze_cpp_msvc;
#else
    constexpr inline bool is_msvc = false;
    constexpr inline int  msvc_version = 0;
#endif

#if defined(raze_cpp_clang)
    constexpr inline bool is_clang = true;
    constexpr inline int  clang_version = raze_cpp_clang;
#else
    constexpr inline bool is_clang = false;
    constexpr inline int  clang_version = 0;
#endif

#if defined(raze_cpp_gnu)
    constexpr inline bool is_gnu = true;
    constexpr inline int  gnu_version = raze_cpp_gnu;
#else
    constexpr inline bool is_gnu = false;
    constexpr inline int  gnu_version = 0;
#endif

#if defined(raze_cpp_mingw)
    constexpr inline bool is_mingw = true;
#else
    constexpr inline bool is_mingw = false;
#endif

#if defined(raze_cpp_llvm_only)
    constexpr inline bool is_llvm_only = true;
#else
    constexpr inline bool is_llvm_only = false;
#endif

#if defined(raze_cpp_msvc_only)
    constexpr inline bool is_msvc_only = true;
#else
    constexpr inline bool is_msvc_only = false;
#endif


#if defined(raze_cpp_clang_only)
    constexpr inline bool is_clang_only = true;
#else
    constexpr inline bool is_clang_only = false;
#endif

#if defined(raze_cpp_gnu_only)
    constexpr inline bool is_gnu_only = true;
#else
    constexpr inline bool is_gnu_only = false;
#endif

#if defined(raze_cpp_icpx_only)
    constexpr inline bool is_icpx_only = true;
#else
    constexpr inline bool is_icpx_only = false;
#endif

__RAZE_COMPATIBILITY_NAMESPACE_END
