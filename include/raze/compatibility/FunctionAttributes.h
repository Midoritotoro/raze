#pragma once

#include <raze/compatibility/CompilerDetection.h>

#if !defined(raze_implicit)
#  if defined(__cpp_conditional_explicit)
#    define raze_implicit explicit(false)
#  else
#    define raze_implicit
#  endif
#endif

#if !defined(raze_noreturn)
#  if defined(raze_cpp_msvc_only)
#    define raze_noreturn __declspec(noreturn)
#  elif defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#    define raze_noreturn __attribute__((__noreturn__))
#  else
#    define raze_noreturn [[noreturn]]
#  endif
#endif // !defined(raze_noreturn)

#if !defined(raze_declare_pure_function)
#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_declare_pure_function __attribute__((pure))
#  else
#    define raze_declare_pure_function
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#endif // !defined(raze_declare_pure_function)


#if !defined(raze_declare_const_function)
#  if defined(raze_cpp_msvc_only)
#    define raze_declare_const_function __declspec(noalias)
#  elif defined(raze_cpp_gnu) || defined(raze_cpp_clang) || defined(raze_cpp_nvhpc)
#    define raze_declare_const_function __attribute__((const))
#  else
#    define raze_declare_const_function
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang) || defined(raze_cpp_msvc)
#endif // !defined(raze_declare_const_function)


#if !defined(raze_declare_cold_function)
#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_declare_cold_function __attribute__((cold))
#  else 
#    define raze_declare_cold_function
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#endif // !defined(raze_declare_cold_function)

#if !defined (raze_unmangled) 
#  define raze_unmangled extern "C"
#endif // !defined(raze_unmangled)
