#pragma once 

#include <raze/compatibility/CompilerDetection.h>

#if (defined(raze_cpp_clang) && raze_cpp_clang < 1600) || defined(raze_cpp_gnu)

#  if !defined(raze_static_operator)
#    define raze_static_operator
#  endif // !defined(raze_static_operator)

#  if !defined(raze_const_operator)
#    define raze_const_operator const
#  endif // !defined(raze_const_operator)

#  if !defined(raze_static_labmda)
#    define raze_static_labmda
#  endif // !defined(raze_static_labmda)

#else

#  if !defined(raze_static_operator)
#    define raze_static_operator static
#  endif // !defined(raze_static_operator)

#  if !defined(raze_const_operator)
#    define raze_const_operator
#  endif // !defined(raze_const_operator)

#  if !defined(raze_static_labmda)
#    define raze_static_labmda static
#  endif // !defined(raze_static_labmda)

#endif // (defined(raze_cpp_clang) && raze_cpp_clang < 1600) || defined(raze_cpp_gnu)