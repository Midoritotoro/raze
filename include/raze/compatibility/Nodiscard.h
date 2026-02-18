#pragma once 

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/LanguageFeatures.h>


#if !defined(raze_has_nodiscard)
#  if !defined(__has_cpp_attribute)
#    define raze_has_nodiscard 0
#  elif __has_cpp_attribute(nodiscard) >= 201603L
#    define raze_has_nodiscard 1
#  else
#    define raze_has_nodiscard 0
#  endif
#endif // !defined(raze_has_nodiscard)


#if !defined(raze_nodiscard)
#  if raze_has_nodiscard
#    define raze_nodiscard  [[nodiscard]]
#  elif defined(raze_cpp_gnu)
#    define raze_nodiscard  __attribute__((__warn_unused_result__))
#  elif defined(raze_cpp_clang)
#    define raze_nodiscard  __attribute__(warn_unused_result)
#  elif defined(raze_cpp_msvc)
#    define raze_nodiscard  _Check_return_
#  else
#    define raze_nodiscard
#  endif // raze_has_nodiscard || defined(raze_cpp_gnu) || defined(raze_cpp_clang) || defined(raze_cpp_msvc)
#endif // !defined(raze_nodiscard)


#if !defined(raze_nodiscard_with_warning)
#  if defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard) >= 201907L
#    define raze_nodiscard_with_warning(message)    [[nodiscard(message)]]
#  elif defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard) >= 201603L
#    define raze_nodiscard_with_warning(message)    raze_nodiscard
#  else
#    define raze_nodiscard_with_warning(message)
#  endif //     defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard) >= 201907L 
//  || defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard) >= 201603L
#endif // !defined(raze_nodiscard_with_warning)


#if !defined(raze_nodiscard_constructor)
// https://open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1771r1.pdf
#  if defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard) >= 201907L
#    define raze_nodiscard_constructor raze_nodiscard
#  else
#    define raze_nodiscard_constructor
#  endif // defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard) >= 201907L
#endif // !defined(raze_nodiscard_constructor)


#if !defined(raze_nodiscard_constructor_with_warning)
#  if defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard) >= 201907L
#    define raze_nodiscard_constructor_with_warning(message) raze_nodiscard_with_warning(message)
#  else
#    define raze_nodiscard_constructor_with_warning(message) 
#  endif // defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard) >= 201907L
#endif // !defined(raze_nodiscard_constructor_with_warning)