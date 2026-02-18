#pragma once 

#include <raze/compatibility/Compatibility.h>

#if defined(raze_processor_x86) && defined(raze_cpp_msvc)

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
# endif // defined(raze_processor_x86) && defined(raze_cpp_msvc)

#if !defined(RAZE_HAS_SSE2_SUPPORT) 
#  if defined(__SSE2__)
#    define RAZE_HAS_SSE2_SUPPORT 1
#  else
#    define RAZE_HAS_SSE2_SUPPORT 0
#  endif // defined(__SSE2__)
#endif // !defined(RAZE_HAS_SSE2_SUPPORT)

#if !defined(RAZE_HAS_SSE3_SUPPORT) 
#  if defined(__SSE3__)
#    define RAZE_HAS_SSE3_SUPPORT 1
#  else
#    define RAZE_HAS_SSE3_SUPPORT 0
#  endif // defined(__SSE3__)
#endif // !defined(RAZE_HAS_SSE3_SUPPORT)

#if !defined(RAZE_HAS_SSSE3_SUPPORT) 
#  if defined(__SSSE3__)
#    define RAZE_HAS_SSSE3_SUPPORT 1
#  else
#    define RAZE_HAS_SSSE3_SUPPORT 0
#  endif // defined(__SSSE3__)
#endif // !defined(RAZE_HAS_SSSE3_SUPPORT)

#if !defined(RAZE_HAS_SSE41_SUPPORT) 
#  if defined (__SSE4_1__)
#    define RAZE_HAS_SSE41_SUPPORT 1
#  else 
#    define RAZE_HAS_SSE41_SUPPORT 0
#  endif // defined(__SSE4_1__)
#endif // !defined(RAZE_HAS_SSE41_SUPPORT)

#if !defined(RAZE_HAS_SSE42_SUPPORT) 
#  if defined (__SSE4_2__)
#    define RAZE_HAS_SSE42_SUPPORT 1
#  else
#    define RAZE_HAS_SSE42_SUPPORT 0
#  endif // defined(__SSE4_2__)
#endif // !defined(RAZE_HAS_SSE42_SUPPORT)

#if !defined(RAZE_HAS_AVX_SUPPORT) 
#  if defined (__AVX__)
#    define RAZE_HAS_AVX_SUPPORT 1
#  else
#    define RAZE_HAS_AVX_SUPPORT 0
#  endif // defined(__AVX__)
#endif // !defined(RAZE_HAS_AVX_SUPPORT)

#if !defined(RAZE_HAS_AVX2_SUPPORT) 
#  if defined (__AVX2__)
#    define RAZE_HAS_AVX2_SUPPORT 1
#  else 
#    define RAZE_HAS_AVX2_SUPPORT 0
#  endif // defined(__AVX2__)
#endif // !defined(RAZE_HAS_AVX2_SUPPORT)

#if !defined(RAZE_HAS_AVX512F_SUPPORT) 
#  if defined (__AVX512F__)
#    define RAZE_HAS_AVX512F_SUPPORT 1
#  else 
#    define RAZE_HAS_AVX512F_SUPPORT 0
#  endif // defined(__AVX512F__)
#endif // !defined(RAZE_HAS_AVX512F_SUPPORT)

#if !defined(RAZE_HAS_AVX512BW_SUPPORT) 
#  if defined (__AVX512BW__)
#    define RAZE_HAS_AVX512BW_SUPPORT 1
#  else 
#    define RAZE_HAS_AVX512BW_SUPPORT 0
#  endif // defined(__AVX512BW__)
#endif // !defined(RAZE_HAS_AVX512BW_SUPPORT)

#if !defined(RAZE_HAS_AVX512CD_SUPPORT) 
#  if defined (__AVX512CD__)
#    define RAZE_HAS_AVX512CD_SUPPORT 1
#  else 
#    define RAZE_HAS_AVX512CD_SUPPORT 0
#  endif // defined(__AVX512CD__)
#endif // !defined(RAZE_HAS_AVX512CD_SUPPORT)

#if !defined(RAZE_HAS_AVX512DQ_SUPPORT) 
#  if defined (__AVX512DQ__)
#    define RAZE_HAS_AVX512DQ_SUPPORT 1
#  else 
#    define RAZE_HAS_AVX512DQ_SUPPORT 0
#  endif // defined(__AVX512DQ__)
#endif // !defined(RAZE_HAS_AVX512DQ_SUPPORT)

#if !defined(RAZE_HAS_AVX512VL_SUPPORT) 
#  if defined (__AVX512VL__)
#    define RAZE_HAS_AVX512VL_SUPPORT 1
#  else 
#    define RAZE_HAS_AVX512VL_SUPPORT 0
#  endif // defined(__AVX512VL__)
#endif // !defined(RAZE_HAS_AVX512VL_SUPPORT)

#if !defined(RAZE_HAS_AVX512VBMI_SUPPORT) 
#  if defined(raze_cpp_msvc)
#    if RAZE_HAS_AVX512F_SUPPORT
#      define RAZE_HAS_AVX512VBMI_SUPPORT 1
#    else 
#      define RAZE_HAS_AVX512VBMI_SUPPORT 0
#    endif // RAZE_HAS_AVX512F_SUPPORT
#  endif // defined(raze_cpp_msvc)
#  else
#    if defined (__AVX512VBMI__)
#      define RAZE_HAS_AVX512VBMI_SUPPORT 1
#    else 
#      define RAZE_HAS_AVX512VBMI_SUPPORT 0
#    endif // defined (__AVX512VBMI__)
#endif // !defined(RAZE_HAS_AVX512VBMI_SUPPORT)

#if !defined(RAZE_HAS_AVX512VBMI2_SUPPORT) 
#  if defined(raze_cpp_msvc)
#    if RAZE_HAS_AVX512F_SUPPORT
#      define RAZE_HAS_AVX512VBMI2_SUPPORT 1
#    else 
#      define RAZE_HAS_AVX512VBMI2_SUPPORT 0
#    endif // RAZE_HAS_AVX512F_SUPPORT
#  endif // defined(raze_cpp_msvc)
#  else
#    if defined (__AVX512VBMI2__)
#      define RAZE_HAS_AVX512VBMI2_SUPPORT 1
#    else 
#      define RAZE_HAS_AVX512VBMI2_SUPPORT 0
#    endif // defined (__AVX512VBMI2__)
#endif // !defined(RAZE_HAS_AVX512VBMI2_SUPPORT)

#if defined(RAZE_FORCE_AVX512F)
#  if !RAZE_HAS_AVX512F_SUPPORT
#    undef RAZE_FORCE_AVX512F
#  endif // !RAZE_HAS_AVX512F_SUPPORT
# endif // defined(RAZE_FORCE_AVX512F)

#if defined(RAZE_FORCE_AVX512BW)
#  if !RAZE_HAS_AVX512BW_SUPPORT
#    undef RAZE_FORCE_AVX512BW
#  endif // !RAZE_HAS_AVX512BW_SUPPORT
# endif // defined(RAZE_FORCE_AVX512BW)

#if defined(RAZE_FORCE_AVX512CD)
#  if !RAZE_HAS_AVX512CD_SUPPORT
#    undef RAZE_FORCE_AVX512CD
#  endif // !RAZE_HAS_AVX512CD_SUPPORT
# endif // defined(RAZE_FORCE_AVX512CD)

#if defined(RAZE_FORCE_AVX512DQ)
#  if !RAZE_HAS_AVX512DQ_SUPPORT
#    undef RAZE_FORCE_AVX512DQ
#  endif // !RAZE_HAS_AVX512DQ_SUPPORT
# endif // defined(RAZE_FORCE_AVX512DQ)

#if defined(RAZE_FORCE_AVX512VL)
#  if !RAZE_HAS_AVX512VL_SUPPORT
#    undef RAZE_FORCE_AVX512VL
#  endif // !RAZE_HAS_AVX512VL_SUPPORT
# endif // defined(RAZE_FORCE_AVX512VL)

#if defined(RAZE_FORCE_AVX512VBMI)
#  if !RAZE_HAS_AVX512VBMI_SUPPORT
#    undef RAZE_FORCE_AVX512VBMI
#  endif // !RAZE_HAS_AVX512VBMI_SUPPORT
# endif // defined(RAZE_FORCE_AVX512VBMI)

#if defined(RAZE_FORCE_AVX512VBMI2)
#  if !RAZE_HAS_AVX512VBMI2_SUPPORT
#    undef RAZE_FORCE_AVX512VBMI2
#  endif // !RAZE_HAS_AVX512VBMI2_SUPPORT
# endif // defined(RAZE_FORCE_AVX512VBMI2)

#if defined(RAZE_FORCE_AVX2)
#  if !RAZE_HAS_AVX2_SUPPORT
#    undef RAZE_FORCE_AVX2
#  endif // !RAZE_HAS_AVX2_SUPPORT
# endif // defined(RAZE_FORCE_AVX2)

#if defined(RAZE_FORCE_AVX)
#  if !RAZE_HAS_AVX_SUPPORT
#    undef RAZE_FORCE_AVX
#  endif // !RAZE_HAS_AVX_SUPPORT
# endif // defined(RAZE_FORCE_AVX)

#if defined(RAZE_FORCE_SSE42)
#  if !RAZE_HAS_SSE42_SUPPORT
#    undef RAZE_FORCE_SSE42
#  endif // !RAZE_HAS_SSE42_SUPPORT
# endif // defined(RAZE_FORCE_SSE42)

#if defined(RAZE_FORCE_SSE41)
#  if !RAZE_HAS_SSE41_SUPPORT
#    undef RAZE_FORCE_SSE41
#  endif // !RAZE_HAS_SSE41_SUPPORT
# endif // defined(RAZE_FORCE_SSE41)

#if defined(RAZE_FORCE_SSSE3)
#  if !RAZE_HAS_SSSE3_SUPPORT
#    undef RAZE_FORCE_SSSE3
#  endif // !RAZE_HAS_SSSE3_SUPPORT
# endif // defined(RAZE_FORCE_SSSE3)

#if defined(RAZE_FORCE_SSE3)
#  if !RAZE_HAS_SSE3_SUPPORT
#    undef RAZE_FORCE_SSE3
#  endif // !RAZE_HAS_SSE3_SUPPORT
# endif // defined(RAZE_FORCE_SSE3)

#if defined(RAZE_FORCE_SSE2)
#  if !RAZE_HAS_SSE2_SUPPORT
#    undef RAZE_FORCE_SSE2
#  endif // !RAZE_HAS_SSE2_SUPPORT
# endif // defined(RAZE_FORCE_SSE2)

#if !defined(RAZE_ISA_FORCE_ENABLED)
#  if defined(RAZE_FORCE_AVX512F) || defined(RAZE_FORCE_AVX512BW) || defined(RAZE_FORCE_AVX512CD) \
	|| defined(RAZE_FORCE_AVX512DQ) || defined(RAZE_FORCE_AVX512VL) || defined(RAZE_FORCE_AVX512VBMI) \
	|| defined(RAZE_FORCE_AVX512VBMI2) || defined(RAZE_FORCE_AVX2) || defined(RAZE_FORCE_AVX) \
	|| defined(RAZE_FORCE_SSE42) || defined(RAZE_FORCE_SSE41) || defined(RAZE_FORCE_SSSE3) \
	|| defined(RAZE_FORCE_SSE3) || defined(RAZE_FORCE_SSE2)
#    define RAZE_ISA_FORCE_ENABLED 1 
#  else 
#    define RAZE_ISA_FORCE_ENABLED 0
#  endif // defined(RAZE_FORCE_AVX512F) || defined(RAZE_FORCE_AVX512BW) || ... || defined(RAZE_FORCE_SSE2).
#endif // defined(RAZE_ISA_FORCE_ENABLED)