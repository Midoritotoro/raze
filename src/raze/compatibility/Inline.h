#pragma once 

#include <src/raze/compatibility/CxxVersionDetection.h>
#include <src/raze/compatibility/CompilerDetection.h>


#if !defined(raze_never_inline)
#  if defined(raze_cpp_msvc_only)
#    define raze_never_inline __declspec(noinline)
#  elif defined(raze_cpp_gnu) || defined(raze_cpp_clang) || defined(raze_cpp_nvhpc)
#    define raze_never_inline __attribute__((noinline))
#  else
#    define raze_never_inline
#  endif // defined(raze_cpp_msvc_only) || defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#endif // !defined(raze_never_inline)


#if !defined(raze_always_inline)
#  if defined(raze_cpp_msvc_only) && defined(NDEBUG)
#    define raze_always_inline __forceinline
#  elif defined(raze_cpp_gnu) || defined(raze_cpp_clang) || defined(raze_cpp_nvhpc)
#    define raze_always_inline inline __attribute__((__always_inline__))
#  else 
#    define raze_always_inline inline
#  endif // defined(raze_cpp_msvc_only) || defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#endif // !defined(raze_always_inline)


#if !defined(raze_always_inline_lambda)
#  if defined(raze_cpp_msvc_only)
#    define raze_always_inline_lambda [[msvc::forceinline]]
#  elif defined(raze_cpp_gnu) || defined(raze_cpp_clang) || defined(raze_cpp_nvhpc)
#    define raze_always_inline_lambda __attribute__((__always_inline__))
#  else 
#    define raze_always_inline_lambda
#  endif // defined(raze_cpp_msvc) || defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#endif // !defined(raze_always_inline_lambda)

#if !defined(raze_no_unique_address)
#  if defined(raze_cpp_msvc)
#    define raze_no_unique_address [[msvc::no_unique_address]]
#  else
#    define raze_no_unique_address [[no_unique_address]]
#  endif // defined(raze_cpp_msvc)
#endif // !defined(raze_no_unique_address)