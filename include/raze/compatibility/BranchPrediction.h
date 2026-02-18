#pragma once 

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/LanguageFeatures.h>


#if !defined(raze_likely)
#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_likely(expression) __builtin_expect(!!(expression), true)
#  elif defined(raze_cpp_msvc) && defined(__has_cpp_attribute) && __has_cpp_attribute(likely) >= 201803L
#    define raze_likely(expression)                     \
       (                                                \
         ([](bool value){                               \
           switch (value) {                             \
             [[unlikely]] case true: return true;       \
             [[likely]] case false: return false;       \
         }                                              \
       })(expression))
#  else
#    define raze_likely(expression) (!!(expression))
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#endif // !defined(raze_likely)


#if !defined(raze_unlikely)
#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_unlikely(expression) __builtin_expect(!!(expression), false)
#  elif defined(raze_cpp_msvc) && defined(__has_cpp_attribute) && __has_cpp_attribute(unlikely) >= 201803L
#    define raze_unlikely(expression)                   \
       (                                                \
         ([](bool value){                               \
           switch (value) {                             \
             [[likely]] case true: return true;         \
             [[unlikely]] case false: return false;     \
         }                                              \
       })(expression))
#  else
#    define raze_unlikely(expression) (!!(expression))
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#endif // !defined(raze_unlikely)


#if !defined(raze_likely_attribute)
#  if defined(__has_cpp_attribute) && __has_cpp_attribute(likely) >= 201803L
#    define raze_likely_attribute [[likely]]
#  else
#    define raze_likely_attribute
#  endif
#endif // !defined(raze_likely_attribute)


#if !defined(raze_unlikely_attribute)
#  if defined(__has_cpp_attribute) && __has_cpp_attribute(unlikely) >= 201803L
#    define raze_unlikely_attribute [[unlikely]]
#  else
#    define raze_unlikely_attribute
#  endif
#endif // !defined(raze_unlikely_attribute)
