#pragma once 

#include <raze/arch/ProcessorDetection.h>


#if !defined(__has_builtin)
#  define __has_builtin(x)             0
#endif // !defined(__has_builtin)


#if !defined(__has_feature)
#  define __has_feature(x)             0
#endif // !defined(__has_feature)


#if !defined(__has_attribute)
#  define __has_attribute(x)           0
#endif // !defined(__has_attribute)


#if !defined(__has_cpp_attribute)
#  define __has_cpp_attribute(x)       0
#endif // !defined(__has_cpp_attribute)


#if !defined(__has_include)
#  define __has_include(x)             0
#endif // !defined(__has_include)


#if !defined(__has_include_next)
#  define __has_include_next(x)        0
#endif // !defined(__has_include_next)


#if defined(__cplusplus)
#  if __has_include(<version>)
#    include <version>
#  endif // __has_include(<version>)
#endif // defined(__cplusplus)
