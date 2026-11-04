#pragma once

#if !defined(RAZE_ECHO)
#  define RAZE_ECHO(...) __VA_ARGS__
#endif

#if defined(__INTEL_LLVM_COMPILER)
#  define raze_cpp_icpx \
      ((((__INTEL_LLVM_COMPILER) / 10000) - 2000) * 100 \
       + (((__INTEL_LLVM_COMPILER) % 10000) / 1000))
#  define raze_cpp_icpx_only raze_cpp_icpx
#elif defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER)
#  define raze_cpp_icc   (__INTEL_COMPILER)
#  define raze_cpp_icc_only raze_cpp_icc
#endif

#if defined(_MSC_VER)
#  define raze_cpp_msvc          (_MSC_VER)

#  if defined(__clang__)
#    undef  raze_cpp_msvc_only
#    define raze_cpp_clang       ((__clang_major__ * 100) + __clang_minor__)

#    if defined(raze_cpp_icpx)  
#      undef  raze_cpp_clang_only 
#    else
#      define raze_cpp_clang_only raze_cpp_clang
#    endif
#  else
#    define raze_cpp_msvc_only   raze_cpp_msvc
#  endif // defined(__clang__)

#elif defined(__GNUC__)
#  define raze_cpp_gnu           (__GNUC__ * 100 + __GNUC_MINOR__)

#  if defined(__MINGW32__)
#    define raze_cpp_mingw
#  endif

#  if defined(__clang__)
#    if defined(__apple_build_version__)
#      if   __apple_build_version__ >= 14030022
#        define raze_cpp_clang 1500
#      elif __apple_build_version__ >= 14000029
#        define raze_cpp_clang 1400
#      elif __apple_build_version__ >= 13160021
#        define raze_cpp_clang 1300
#      elif __apple_build_version__ >= 13000029
#        define raze_cpp_clang 1200
#      elif __apple_build_version__ >= 12050022
#        define raze_cpp_clang 1110
#      elif __apple_build_version__ >= 12000032
#        define raze_cpp_clang 1000
#      elif __apple_build_version__ >= 11030032
#        define raze_cpp_clang 900
#      elif __apple_build_version__ >= 11000033
#        define raze_cpp_clang 800
#      else
#        error "Unsupported Apple Clang version"
#      endif
#    else
#      define raze_cpp_clang       ((__clang_major__ * 100) + __clang_minor__)
#    endif // __apple_build_version__

#    if defined(raze_cpp_icpx)
#      undef  raze_cpp_clang_only 
#    else
#      define raze_cpp_clang_only raze_cpp_clang
#      define raze_cpp_clang_pure raze_cpp_clang
#    endif

#    if !defined(__has_extension)
#      define __has_extension __has_feature
#    endif
#  else
#    if defined(raze_cpp_icc)
#      define raze_cpp_gnu_only   raze_cpp_gnu   
#    else
#      define raze_cpp_gnu_only   raze_cpp_gnu
#    endif
#  endif // __clang__
#elif defined(__clang__) || defined(__llvm__)
#  if defined(__clang__)
#    define raze_cpp_clang         ((__clang_major__ * 100) + __clang_minor__)
#    if !defined(__has_extension)
#      define __has_extension __has_feature
#    endif
#  endif

#  if defined(raze_cpp_icpx)
#    undef  raze_cpp_clang_only
#  elif defined(__clang__)
#    define raze_cpp_clang_only    raze_cpp_clang
#    define raze_cpp_clang_pure    raze_cpp_clang
#  else
#    define raze_cpp_llvm_only
#  endif

#endif