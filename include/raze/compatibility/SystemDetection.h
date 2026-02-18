#pragma once

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  define raze_os_apple
#  if defined(TARGET_OS_MAC) && TARGET_OS_MAC
#    define raze_os_darwin
#    define raze_os_bsd4
#    if defined(OS_IPHONE) && TARGET_OS_IPHONE
#    else
#      define raze_os_mac
#    endif
#  endif
#elif defined(__CYGWIN__)
#  define raze_os_cygwin
#elif !defined(SAG_COM) && (!defined(WINAPI_FAMILY) || WINAPI_FAMILY==WINAPI_FAMILY_DESKTOP_APP) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define raze_os_win32
#  define raze_os_win64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  define raze_os_win32
#elif defined(__linux__) || defined(__linux)
#  define raze_os_linux

#elif defined(__Lynx__)
#  define raze_os_lynx

#elif defined(__GNU__)
#  define raze_os_hurd

#elif defined(__FreeBSD__)
#  define raze_os_freebsd

#elif defined(__NetBSD__)
#  define raze_os_netbsd

#elif defined(__OpenBSD__)
#  define raze_os_openbsd

#elif defined(__DragonFly__)
#  define raze_os_dragonfly

#elif defined(__linux__)
#  define raze_os_linux

#elif defined(__native_client__)
#  define raze_os_nacl

#elif defined(__EMSCRIPTEN__)
#  define raze_os_emscripten

#elif defined(__rtems__)
#  define raze_os_rtems

#elif defined(__Fuchsia__)
#  define raze_os_fuchsia

#elif defined (__SVR4) && defined (__sun)
#  define raze_os_solaris

#elif defined(__QNX__)
#  define raze_os_qnx

#elif defined(__MVS__)
#  define raze_os_zos

#elif defined(__hexagon__)
#  define raze_os_qurt

#else
#  error ""
#endif

#if defined(raze_os_win32) || defined(raze_os_win64)
#  define raze_os_windows
#  define raze_os_win
#endif

#if defined(raze_os_windows)
#  undef raze_os_unix
#elif !defined(raze_os_unix)
#  define raze_os_unix
#endif

#if defined(raze_os_win)
#  define NOMINMAX
#endif // defined(raze_os_win)

#if defined(raze_os_windows)
#  include <windows.h>
#  include <stdnoreturn.h>
#endif // defined(raze_os_windows)

#if defined(max) 
#  undef max
#endif

#if defined(min) 
#  undef min
#endif