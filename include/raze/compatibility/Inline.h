#pragma once 

#include <raze/compatibility/CxxVersionDetection.h>
#include <raze/compatibility/CompilerDetection.h>


#if !defined(raze_never_inline)
#  if defined(raze_cpp_msvc) || defined(raze_cpp_clang)
#     define raze_never_inline __declspec(noinline)
#  elif defined(raze_cpp_gnu) 
#    define raze_never_inline __attribute__((noinline))
#  else 
#    define raze_never_inline 
#  endif // defined(raze_cpp_msvc) || defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#endif // !defined(raze_never_inline)


#if !defined(raze_always_inline)
#  if defined(raze_cpp_msvc)
#    define raze_always_inline __forceinline
#  elif defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_always_inline inline __attribute__((always_inline))
#  else 
#    define raze_always_inline inline
#  endif // defined(raze_cpp_msvc) || defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#endif // !defined(raze_always_inline)


#if !defined(raze_constexpr_cxx20)
#  if raze_has_cxx20
#    define raze_constexpr_cxx20 constexpr
#  else
#    define raze_constexpr_cxx20
#  endif // raze_has_cxx20
#endif // !defined(raze_constexpr_cxx20)


#if !defined(raze_clang_constexpr_cxx20)
#  if defined(raze_cpp_clang)
#    define raze_clang_constexpr_cxx20 raze_constexpr_cxx20
#  else
#    define raze_clang_constexpr_cxx20 
#  endif // defined(raze_cpp_clang)
#endif // !defined(raze_clang_constexpr_cxx20)