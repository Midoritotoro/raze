#pragma once 

#include <raze/compatibility/CompilerDetection.h>


#if !defined(raze_targets)
#  if defined(raze_cpp_gnu) || (defined(raze_cpp_clang) && raze_cpp_clang >= 1500)
#    define raze_targets(...) [[gnu::target_clones(__VA_ARGS__)]]
#  else
#    define raze_targets(...)
#  endif // defined(raze_cpp_gnu) || (defined(raze_cpp_clang) && raze_cpp_clang >= 1500)
#endif // !defined(raze_targets)
