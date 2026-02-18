#pragma once 

#include <raze/compatibility/CompilerDetection.h>
#include <raze/arch/ProcessorDetection.h>

#include <raze/compatibility/SimdCompatibility.h>


#if !defined(raze_fastcall)
#  if defined(raze_processor_x86_32)
#    if defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#      define raze_fastcall __attribute__((regparm(3)))
#    elif defined(raze_cpp_msvc)
#      define raze_fastcall __fastcall
#    else
#      define raze_fastcall
#    endif // defined(raze_cpp_gnu) || defined(raze_cpp_msvc) || defined(raze_cpp_clang)
#  else
#    define raze_fastcall
#  endif // defined(raze_processor_x86_32)
#endif // !defined(raze_fastcall)


#if !defined(raze_stdcall)
#  if defined(raze_cpp_msvc) || defined(raze_cpp_clang)
#    define raze_stdcall            __stdcall
#  elif defined(raze_cpp_gnu)
#    define raze_stdcall            __attribute__((__stdcall__))
#  else
#    define raze_stdcall        
#  endif // defined(raze_cpp_msvc) || defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#endif // !defined(raze_stdcall)


#if !defined(raze_cdecl)
#  if defined(raze_cpp_msvc) || defined(raze_cpp_clang)
#    define raze_cdecl          __cdecl
#  elif defined(raze_cpp_gnu)
#    define raze_cdecl          __attribute__((__cdecl__))
#  else
#    define raze_cdecl        
#  endif // defined(raze_cpp_msvc) || defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#endif // !defined(raze_cdecl)


#if !defined(raze_vectorcall)
#  if defined(raze_cpp_msvc) && defined(raze_processor_x86) && defined(__SSE2__)
#    define raze_vectorcall __vectorcall
#  else
#    define raze_vectorcall
#  endif
#endif // !defined(raze_vectorcall)
