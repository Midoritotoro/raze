#pragma once

#include <raze/compatibility/CompilerDetection.h>


#if defined(raze_cpp_msvc) || defined(raze_cpp_clang)
#  include <stdnoreturn.h> 
#endif // defined(raze_cpp_msvc) || defined(raze_cpp_clang)


#if defined(__cpp_conditional_explicit)
#  if !defined(raze_implicit)
#    define raze_implicit explicit(false)
#  else
#    define raze_implicit
#  endif // raze_implicit
#endif // defined(__cpp_conditional_explicit)


#if !defined(raze_noreturn)
#  if defined(raze_cpp_gnu)
#    define raze_noreturn           __attribute__((__noreturn__))
#  elif defined(raze_cpp_msvc) || defined(raze_cpp_clang)
#    define raze_noreturn           __declspec(noreturn)
#  else 
#    define raze_noreturn       
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_msvc) || defined(raze_cpp_clang)
#endif // !defined(raze_noreturn)


#if !defined(raze_declare_pure_function)
#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_declare_pure_function __attribute__((pure))
#  else
#    define raze_declare_pure_function
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#endif // !defined(raze_declare_pure_function)


#if !defined(raze_declare_const_function)
#  if defined(raze_cpp_msvc)
#    define raze_declare_const_function __declspec(noalias)
#  elif defined(raze_cpp_gnu) || defined(raze_cpp_clang)
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
