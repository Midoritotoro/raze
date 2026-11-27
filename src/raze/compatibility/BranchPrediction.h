#pragma once 

#include <src/raze/compatibility/CompilerDetection.h>
#include <src/raze/compatibility/LanguageFeatures.h>


#if !defined(raze_likely)
#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_likely(expression) __builtin_expect(!!(expression), true)
#  elif defined(raze_cpp_msvc) && defined(__has_cpp_attribute) && __has_cpp_attribute(likely) >= 201803L
#    define raze_likely(expression)                                 \
       (                                                            \
         ([](bool value) noexcept {                                 \
           if (value) [[likely]] return true;                       \
           else [[unlikely]] return false;                          \
         })(static_cast<bool>(expression)))
#  else
#    define raze_likely(expression) (!!(expression))
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#endif // !defined(raze_likely)


#if !defined(raze_unlikely)
#  if defined(raze_cpp_gnu) || defined(raze_cpp_clang)
#    define raze_unlikely(expression) __builtin_expect(!!(expression), false)
#  elif defined(raze_cpp_msvc) && defined(__has_cpp_attribute) && __has_cpp_attribute(unlikely) >= 201803L
#    define raze_unlikely(expression)                               \
       (                                                            \
         ([](bool value) noexcept {                                 \
           if (value) [[unlikely]] return true;                     \
           else [[likely]] return false;                            \
         })(static_cast<bool>(expression)))
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


#if !defined(raze_assume) 
#  if defined(raze_cpp_msvc_only)
#    define raze_assume(__cond) __assume(__cond)
#  elif defined(raze_cpp_clang)
#    define raze_assume(__cond) __builtin_assume(__cond)
#  elif defined(raze_cpp_gnu)
#    if raze_cpp_gnu >= 1400
#      define raze_assume(__cond) __attribute__((assume(__cond)))
#    else
#      define raze_assume(__cond)
#    endif
#  else
#    define raze_assume(__cond)
#  endif // defined(raze_cpp_msvc) || defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#endif // !defined(raze_assume)