#pragma once 

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/Warnings.h>


#if defined(RAZE_ENABLE_OPENMP)
#  if !defined(_OPENMP)
#    undef RAZE_ENABLE_OPENMP
#  endif // !defined(_OPENMP)
#endif // defined(RAZE_ENABLE_OPENMP)

#if !defined(raze_vectorize)
#  if defined(RAZE_ENABLE_OPENMP)
#    define raze_vectorize raze_do_pragma(omp simd)
#  elif defined(raze_cpp_gnu) && raze_cpp_gnu >= 490
#    define raze_vectorize raze_do_pragma(GCC ivdep)
#  elif defined(raze_cpp_clang)
#    define raze_vectorize raze_do_pragma(clang loop vectorize(enable))
#  else
#    define raze_vectorize
#  endif // defined(RAZE_ENABLE_OPENMP)
#endif // !defined(raze_vectorize)

