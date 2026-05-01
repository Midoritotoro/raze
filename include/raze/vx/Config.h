#pragma once 

#include <raze/compatibility/Compatibility.h>

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

#if !defined(RAZE_HAS_FMA3_SUPPORT)
#  if defined(__FMA3__)
#    define RAZE_HAS_FMA3_SUPPORT 1
#  else
#    define RAZE_HAS_FMA3_SUPPORT 0
#  endif // defined(__FMA3__)
#endif // !defined(RAZE_HAS_FMA3_SUPPORT)

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

#if defined(RAZE_FORCE_FMA3)
#  if !RAZE_HAS_FMA3_SUPPORT
#    undef RAZE_FORCE_FMA3
#  endif // !RAZE_HAS_FMA3_SUPPORT
# endif // defined(RAZE_FORCE_FMA3)

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

#if !defined(__raze_simd_algorithm_inline)
#  if defined(RAZE_ISA_FORCE_ENABLED)
#    define __raze_simd_algorithm_inline raze_always_inline
#  else 
#    define __raze_simd_algorithm_inline raze_never_inline
#  endif // defined(RAZE_ISA_FORCE_ENABLED)
#endif // !defined(__raze_simd_algorithm_inline)

__RAZE_VX_NAMESPACE_BEGIN

constexpr arch::ISA __best_isa_compile_time() noexcept {
#if defined(RAZE_FORCE_AVX512VBMI2VLDQ) && RAZE_HAS_AVX512VBMI2_SUPPORT && RAZE_HAS_AVX512VL_SUPPORT && RAZE_HAS_AVX512DQ_SUPPORT && RAZE_HAS_AVX512BW_SUPPORT
    return arch::ISA::AVX512VBMI2VLDQ;
#elif defined(RAZE_FORCE_AVX512VBMIVLDQ) && RAZE_HAS_AVX512VBMI_SUPPORT && RAZE_HAS_AVX512VL_SUPPORT && RAZE_HAS_AVX512DQ_SUPPORT && RAZE_HAS_AVX512BW_SUPPORT
    return arch::ISA::AVX512VBMIVLDQ;
#elif defined(RAZE_FORCE_AVX512VBMI2DQ) && RAZE_HAS_AVX512VBMI2_SUPPORT && RAZE_HAS_AVX512DQ_SUPPORT && RAZE_HAS_AVX512BW_SUPPORT
    return arch::ISA::AVX512VBMI2DQ;
#elif defined(RAZE_FORCE_AVX512VBMIDQ) && RAZE_HAS_AVX512VBMI_SUPPORT && RAZE_HAS_AVX512DQ_SUPPORT && RAZE_HAS_AVX512BW_SUPPORT
    return arch::ISA::AVX512VBMIDQ;
#elif defined(RAZE_FORCE_AVX512VBMI2VL) && RAZE_HAS_AVX512VBMI2_SUPPORT && RAZE_HAS_AVX512VL_SUPPORT
    return arch::ISA::AVX512VBMI2VL;
#elif defined(RAZE_FORCE_AVX512VBMIVL) && RAZE_HAS_AVX512VBMI_SUPPORT && RAZE_HAS_AVX512VL_SUPPORT
    return arch::ISA::AVX512VBMIVL;
#elif defined(RAZE_FORCE_AVX512VLBWDQ) && RAZE_HAS_AVX512VL_SUPPORT && RAZE_HAS_AVX512BW_SUPPORT && RAZE_HAS_AVX512DQ_SUPPORT
    return arch::ISA::AVX512VLBWDQ;
#elif defined(RAZE_FORCE_AVX512VLDQ) && RAZE_HAS_AVX512VL_SUPPORT && RAZE_HAS_AVX512DQ_SUPPORT
    return arch::ISA::AVX512VLDQ;
#elif defined(RAZE_FORCE_AVX512VLBW) && RAZE_HAS_AVX512VL_SUPPORT && RAZE_HAS_AVX512BW_SUPPORT
    return arch::ISA::AVX512VLBW;
#elif defined(RAZE_FORCE_AVX512VLF) && RAZE_HAS_AVX512VL_SUPPORT && RAZE_HAS_AVX512F_SUPPORT
    return arch::ISA::AVX512VLF;
#elif defined(RAZE_FORCE_AVX512BWDQ) && RAZE_HAS_AVX512BW_SUPPORT && RAZE_HAS_AVX512DQ_SUPPORT
    return arch::ISA::AVX512BWDQ;
#elif defined(RAZE_FORCE_AVX512VBMI2) && RAZE_HAS_AVX512VBMI2_SUPPORT
    return arch::ISA::AVX512VBMI2;
#elif defined(RAZE_FORCE_AVX512VBMI) && RAZE_HAS_AVX512VBMI_SUPPORT
    return arch::ISA::AVX512VBMI;
#elif defined(RAZE_FORCE_AVX512DQ) && RAZE_HAS_AVX512DQ_SUPPORT
    return arch::ISA::AVX512DQ;
#elif defined(RAZE_FORCE_AVX512BW) && RAZE_HAS_AVX512BW_SUPPORT
    return arch::ISA::AVX512BW;
#elif defined(RAZE_FORCE_AVX512F) && RAZE_HAS_AVX512F_SUPPORT
    return arch::ISA::AVX512F;
#elif defined(RAZE_FORCE_AVX2FMA3) && RAZE_HAS_AVX2_SUPPORT && RAZE_HAS_FMA3_SUPPORT
    return arch::ISA::AVX2FMA3;
#elif defined(RAZE_FORCE_AVX2) && RAZE_HAS_AVX2_SUPPORT
    return arch::ISA::AVX2;
#elif defined(RAZE_FORCE_FMA3) && RAZE_HAS_FMA3_SUPPORT
    return arch::ISA::FMA3;
#elif defined(RAZE_FORCE_AVX) && RAZE_HAS_AVX_SUPPORT
    return arch::ISA::AVX;
#elif defined(RAZE_FORCE_SSE42) && RAZE_HAS_SSE42_SUPPORT
    return arch::ISA::SSE42;
#elif defined(RAZE_FORCE_SSE41) && RAZE_HAS_SSE41_SUPPORT
    return arch::ISA::SSE41;
#elif defined(RAZE_FORCE_SSSE3) && RAZE_HAS_SSSE3_SUPPORT
    return arch::ISA::SSSE3;
#elif defined(RAZE_FORCE_SSE3) && RAZE_HAS_SSE3_SUPPORT
    return arch::ISA::SSE3;
#elif defined(RAZE_FORCE_SSE2) && RAZE_HAS_SSE2_SUPPORT
    return arch::ISA::SSE2;
#elif RAZE_HAS_SSE2_SUPPORT
    return arch::ISA::SSE2;
#else
    return arch::ISA::None;
#endif
}

__RAZE_VX_NAMESPACE_END
