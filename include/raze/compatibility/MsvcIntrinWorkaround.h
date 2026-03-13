#pragma once 

#include <raze/compatibility/Inline.h>

#if !defined(__raze_msvc_intrin_workaround)
#  if defined(raze_cpp_msvc)
#    define __raze_msvc_intrin_workaround raze_never_inline
#  else
#    define __raze_msvc_intrin_workaround raze_always_inline
#  endif
#endif // !defined(__raze_msvc_intrin_workaround)
