#pragma once 

#include <raze/compatibility/Warnings.h>

#if !defined(raze_disable_unrolling)
#  if defined(raze_cpp_msvc_only)
#    define raze_disable_unrolling // raze_do_pragma(loop(no_vector))
#  elif defined(raze_cpp_clang) 
#    define raze_disable_unrolling \
       raze_do_pragma(clang loop unroll(disable)) \
       raze_do_pragma(clang loop vectorize(disable)) \
       raze_do_pragma(clang loop interleave(disable))
#  elif defined(raze_cpp_gnu)
#    define raze_disable_unrolling raze_do_pragma(GCC unroll 1) \
        raze_do_pragma(GCC optimize("no-tree-vectorize"))
#  endif // defined(raze_cpp_msvc_only)
#endif // !defined(raze_disable_unrolling)
