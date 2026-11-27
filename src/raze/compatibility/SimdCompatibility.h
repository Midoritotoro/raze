#pragma once

#include <src/raze/compatibility/CompilerDetection.h>
#include <src/raze/arch/ProcessorDetection.h>

#if defined(raze_processor_x86) && defined(raze_cpp_msvc_only)

#  if (defined(_M_X64) || _M_IX86_FP >= 2)
#    define __SSE__ 1
#    define __SSE2__ 1
#  endif // (defined(_M_X64) || _M_IX86_FP >= 2)

#  if (defined(_M_AVX) || defined(__AVX__))
#    define __SSE3__                        1

#    define __SSSE3__                       1
#    define __SSE4_1__                      1

#    define __SSE4_2__                      1
#    define __POPCNT__                      1

#    if !defined(__AVX__)
#      define __AVX__                       1
#    endif // !defined(__AVX__)

#  endif // (defined(_M_AVX) || defined(__AVX__))
#  ifdef __AVX2__
// MSVC defines __AVX2__ with /arch:AVX2
#    define __F16C__                        1
#    define __RDRND__                       1

#    define __FMA__                         1

#    define __BMI__                         1
#    define __BMI2__                        1

#    define __MOVBE__                       1
#    define __LZCNT__                       1
#  endif // __AVX2__
#endif // defined(raze_processor_x86) && defined(raze_cpp_msvc_only)

#if defined(raze_cpp_msvc) && !defined(_M_ARM64EC)
#  include <intrin.h>
#endif

#if defined(raze_processor_x86) && defined(__SSE2__)
#  include <emmintrin.h>
#endif // defined(raze_processor_x86) && defined(__SSE2__)

#if defined(raze_processor_x86) && defined(raze_cpp_nvhpc)
#  include <immintrin.h>
#  include <smmintrin.h>
#  include <avxintrin.h>
#  include <avx2intrin.h>
#  include <f16cintrin.h>
#  include <fmaintrin.h>
#  include <bmiintrin.h>
#  include <bmi2intrin.h>
#  include <lzcntintrin.h>
#  include <popcntintrin.h>
#  include <avx512fintrin.h>
#  include <avx512vlintrin.h>
#  include <avx512bwintrin.h>
#  include <avx512vlbwintrin.h>
#  include <avx512dqintrin.h>
#  include <avx512vldqintrin.h>
#  include <avx512cdintrin.h>
#  include <avx512vlcdintrin.h>
#  include <avx512vbmiintrin.h>
#  include <avx512vbmivlintrin.h>
#  include <avx512vbmi2intrin.h>
#  include <avx512vlvbmi2intrin.h>
#endif // defined(raze_processor_x86) && defined(raze_cpp_nvhpc)