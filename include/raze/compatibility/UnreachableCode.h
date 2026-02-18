#pragma once 

#include <raze/compatibility/CompilerDetection.h>


#if !defined(raze_maybe_unused_attribute)
#  if defined(__has_cpp_attribute) && __has_cpp_attribute(maybe_unused) >= 201603L
#    define raze_maybe_unused_attribute     [[maybe_unused]]
#  elif defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_maybe_unused_attribute     __attribute__(unused)
#  endif // defined(__has_cpp_attribute) && __has_cpp_attribute(maybe_unused) >= 201603L
#endif // !defined(raze_maybe_unused_attribute)


#if !defined(raze_unused)
#  define raze_unused(variable) ((void)(variable))
#endif // !defined(raze_unused)

#if !defined(raze_unreachable)
#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_unreachable() __builtin_unreachable()
#  elif defined(raze_cpp_msvc)
#    define raze_unreachable() (__assume(0))
#  else
#    define raze_unreachable() ((void)0)
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang) || defined(raze_cpp_msvc)
#endif // !defined(raze_unreachable)

