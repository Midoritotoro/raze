#pragma once

#include <src/raze/compatibility/CompilerDetection.h>
#include <src/raze/arch/ProcessorDetection.h>
#include <src/raze/compatibility/SimdCompatibility.h>

#if !defined(raze_fastcall)
#  if defined(raze_processor_x86_32)
#    if defined(raze_cpp_msvc)
#      define raze_fastcall __fastcall
#    elif defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#      define raze_fastcall __attribute__((regparm(3)))
#    else
#      define raze_fastcall
#    endif
#  else
#    define raze_fastcall
#  endif
#endif // !defined(raze_fastcall)

#if !defined(raze_stdcall)
#  if defined(raze_cpp_msvc)
#    define raze_stdcall __stdcall
#  elif defined(raze_processor_x86_32) && (defined(raze_cpp_gnu) || defined(raze_cpp_clang))
#    define raze_stdcall __attribute__((__stdcall__))
#  else
#    define raze_stdcall
#  endif
#endif // !defined(raze_stdcall)

#if !defined(raze_cdecl)
#  if defined(raze_cpp_msvc)
#    define raze_cdecl __cdecl
#  elif defined(raze_processor_x86_32) && (defined(raze_cpp_gnu) || defined(raze_cpp_clang))
#    define raze_cdecl __attribute__((__cdecl__))
#  else
#    define raze_cdecl
#  endif
#endif // !defined(raze_cdecl)

#if !defined(raze_vectorcall)
#  if defined(raze_cpp_msvc) && defined(raze_processor_x86) && defined(__SSE2__)
#    define raze_vectorcall __vectorcall
#  else
#    define raze_vectorcall
#  endif
#endif // !defined(raze_vectorcall)