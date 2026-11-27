#pragma once 

// Clang attributes
// https://clang.llvm.org/docs/AttributeReference.html#always-inline-force-inline
// Clang builtins
// https://clang.llvm.org/docs/LanguageExtensions.html

// Msvc attributes
// https://learn.microsoft.com/en-us/cpp/cpp/declspec?view=msvc-170

// Gcc attributes
// https://ohse.de/uwe/articles/gcc-attributes.html and https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html

#include <src/raze/RazeNamespace.h>
#include <src/raze/compatibility/CompilerDetection.h>
#include <src/raze/compatibility/CxxVersionDetection.h>
#include <src/raze/compatibility/SystemDetection.h>
#include <src/raze/compatibility/LanguageFeatures.h>
#include <src/raze/compatibility/AlignmentMacros.h>
#include <src/raze/compatibility/BranchPrediction.h>
#include <src/raze/compatibility/CallingConventions.h>
#include <src/raze/compatibility/Inline.h>
#include <src/raze/compatibility/FunctionAttributes.h>
#include <src/raze/compatibility/MemoryMacros.h>
#include <src/raze/compatibility/Nodiscard.h>
#include <src/raze/compatibility/SimdCompatibility.h>
#include <src/raze/compatibility/UnreachableCode.h>
#include <src/raze/compatibility/Warnings.h>
#include <src/raze/compatibility/StaticOperators.h>
#include <src/raze/compatibility/StackProtection.h>
#include <src/raze/compatibility/Unroll.h>
#include <src/raze/compatibility/TargetClones.h>

#include <cstddef>
#include <string_view>

raze_disable_warning_msvc(4067)

__RAZE_COMPATIBILITY_NAMESPACE_BEGIN

namespace detail {

consteval bool detect_aocc() noexcept {
#if defined(raze_cpp_clang) && !defined(raze_cpp_icpx)
#  if defined(__clang_version__)
    if (std::string_view(__clang_version__).contains("AOCC"))
        return true;
#  endif
#  if defined(__VERSION__)
    if (std::string_view(__VERSION__).contains("AOCC"))
        return true;
#  endif
#endif
    return false;
}

consteval int parse_aocc_version(std::string_view str) noexcept {
    const auto pos = str.find("AOCC_");
    if (pos == std::string_view::npos)
        return 0;

    std::size_t i = pos + 5;
    int major = 0;
    bool has_major = false;
    while (i < str.size() && str[i] >= '0' && str[i] <= '9') {
        major = major * 10 + (str[i] - '0');
        has_major = true;
        ++i;
    }
    if (!has_major)
        return 0;

    int minor = 0;
    if (i < str.size() && str[i] == '.') {
        ++i;
        while (i < str.size() && str[i] >= '0' && str[i] <= '9') {
            minor = minor * 10 + (str[i] - '0');
            ++i;
        }
    }
    return major * 100 + minor;
}

consteval int detect_aocc_version() noexcept {
#if defined(raze_cpp_clang) && !defined(raze_cpp_icpx)
    if constexpr (detect_aocc()) {
#  if defined(__clang_version__)
        if (const int v = parse_aocc_version(__clang_version__); v != 0)
            return v;
#  endif
#  if defined(__VERSION__)
        if (const int v = parse_aocc_version(__VERSION__); v != 0)
            return v;
#  endif
        return raze_cpp_clang;
    }
#endif
    return 0;
}

} // namespace detail

// C++ standard versions
constexpr inline bool is_cxx11 = (raze_has_cxx11 == 1);
constexpr inline bool is_cxx14 = (raze_has_cxx14 == 1);
constexpr inline bool is_cxx17 = (raze_has_cxx17 == 1);
constexpr inline bool is_cxx20 = (raze_has_cxx20 == 1);
constexpr inline bool is_cxx23 = (raze_has_cxx23 == 1);

// Operating systems
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

// Compilers (broad family flags)
#if defined(raze_cpp_nvhpc)
    constexpr inline bool is_nvhpc = true;
    constexpr inline int  nvhpc_version = raze_cpp_nvhpc;
#else
    constexpr inline bool is_nvhpc = false;
    constexpr inline int  nvhpc_version = 0;
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

constexpr inline bool is_aocc = detail::detect_aocc();
constexpr inline int  aocc_version = detail::detect_aocc_version();

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

#if defined(raze_cpp_nvhpc_only)
    constexpr inline bool is_nvhpc_only = true;
#else
    constexpr inline bool is_nvhpc_only = false;
#endif

#if defined(raze_cpp_icpx_only)
    constexpr inline bool is_icpx_only = true;
#else
    constexpr inline bool is_icpx_only = false;
#endif

#if defined(raze_cpp_icc_only)
    constexpr inline bool is_icc_only = true;
#else
    constexpr inline bool is_icc_only = false;
#endif

constexpr inline bool is_aocc_only = is_aocc;

#if defined(raze_cpp_msvc_only)
    constexpr inline bool is_msvc_only = true;
#else
    constexpr inline bool is_msvc_only = false;
#endif

#if defined(raze_cpp_clang_only)
    constexpr inline bool is_clang_only = !is_aocc;
#else
    constexpr inline bool is_clang_only = false;
#endif

#if defined(raze_cpp_clang_pure)
    constexpr inline bool is_clang_pure = !is_aocc;
#else
    constexpr inline bool is_clang_pure = false;
#endif

#if defined(raze_cpp_gnu_only)
    constexpr inline bool is_gnu_only = true;
#else
    constexpr inline bool is_gnu_only = false;
#endif

#if defined(raze_cpp_llvm_only)
    constexpr inline bool is_llvm_only = true;
#else
    constexpr inline bool is_llvm_only = false;
#endif

// Convenience derived flags
constexpr inline bool is_clang_cl = is_msvc && is_clang && !is_icpx && !is_aocc;
constexpr inline bool is_static_isa_compiler = is_gnu || is_clang || is_nvhpc || is_icc;

__RAZE_COMPATIBILITY_NAMESPACE_END