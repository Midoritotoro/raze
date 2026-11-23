#pragma once

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/LanguageFeatures.h>

#if defined(__cpp_static_call_operator) && __cpp_static_call_operator >= 202207L

#  if !defined(raze_static_operator)
#    define raze_static_operator static
#  endif

#  if !defined(raze_const_operator)
#    define raze_const_operator
#  endif

#  if !defined(raze_static_lambda)
#    define raze_static_lambda static
#  endif

#else

#  if !defined(raze_static_operator)
#    define raze_static_operator
#  endif

#  if !defined(raze_const_operator)
#    define raze_const_operator const
#  endif

#  if !defined(raze_static_lambda)
#    define raze_static_lambda
#  endif

#endif // defined(__cpp_static_call_operator) && __cpp_static_call_operator >= 202207L
