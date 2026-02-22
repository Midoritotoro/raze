#pragma once 

#include <raze/datapar/BasicSimd.h>
#include <raze/datapar/SimdConfig.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <typename _Type_>
using simd128_sse2			= simd<arch::ISA::SSE2, _Type_, 128>;

template <typename _Type_>
using simd128_sse3			= simd<arch::ISA::SSE3, _Type_, 128>;

template <typename _Type_>
using simd128_ssse3			= simd<arch::ISA::SSSE3, _Type_, 128>;

template <typename _Type_>
using simd128_sse41			= simd<arch::ISA::SSE41, _Type_, 128>;

template <typename _Type_>
using simd128_sse42			= simd<arch::ISA::SSE42, _Type_, 128>;

template <typename _Type_>
using simd128_avx512vlbw	= simd<arch::ISA::AVX512VLBW, _Type_, 128>;

template <typename _Type_>
using simd128_avx512vlf		= simd<arch::ISA::AVX512VLF, _Type_, 128>;

template <typename _Type_>
using simd128_avx512vldq	= simd<arch::ISA::AVX512VLDQ, _Type_, 128>;


template <typename _Type_>
using simd256_avx			= simd<arch::ISA::AVX, _Type_, 256>;

template <typename _Type_>
using simd256_avx2			= simd<arch::ISA::AVX2, _Type_, 256>;

template <typename _Type_>
using simd256_avx512vlbw	= simd<arch::ISA::AVX512VLBW, _Type_, 256>;

template <typename _Type_>
using simd256_avx512vlf		= simd<arch::ISA::AVX512VLF, _Type_, 256>;

template <typename _Type_>
using simd256_avx512vldq	= simd<arch::ISA::AVX512VLDQ, _Type_, 256>;

template <typename _Type_>
using simd256_avx512vlbwdq	= simd<arch::ISA::AVX512VLBWDQ, _Type_, 256>;


template <typename _Type_>
using simd512_avx512f		= simd<arch::ISA::AVX512F, _Type_, 512>;

template <typename _Type_>
using simd512_avx512bw		= simd<arch::ISA::AVX512BW, _Type_, 512>;

template <typename _Type_>
using simd512_avx512dq		= simd<arch::ISA::AVX512DQ, _Type_, 512>;

template <typename _Type_>
using simd512_avx512bwdq	= simd<arch::ISA::AVX512BWDQ, _Type_, 512>;


template <typename _Type_>
#if defined(RAZE_HAS_AVX512VBMI2_SUPPORT) 
  using simd_native = simd<arch::ISA::AVX512VBMI2, _Type_, 512>;

#elif defined(RAZE_HAS_AVX512VBMI_SUPPORT)
  using simd_native = simd<arch::ISA::AVX512VBMI, _Type_, 512>;

#elif defined(RAZE_HAS_AVX512DQ_SUPPORT) && defined(RAZE_HAS_AVX512BW_SUPPORT)
  using simd_native = simd<arch::ISA::AVX512BWDQ, _Type_, 512>;

#elif defined(RAZE_HAS_AVX512VL_SUPPORT) && defined(RAZE_HAS_AVX512BW_SUPPORT) && defined(RAZE_HAS_AVX512DQ_SUPPORT)
  using simd_native = simd<arch::ISA::AVX512VLBWDQ, _Type_, 256>;

#elif defined(RAZE_HAS_AVX512VL_SUPPORT) && defined(RAZE_HAS_AVX512BW_SUPPORT)
  using simd_native = simd<arch::ISA::AVX512VLBW, _Type_, 256>;

#elif defined(RAZE_HAS_AVX512VL_SUPPORT) && defined(RAZE_HAS_AVX512DQ_SUPPORT)
  using simd_native = simd<arch::ISA::AVX512VLDQ, _Type_, 256>;

#elif defined(RAZE_HAS_AVX512VL_SUPPORT) && defined(RAZE_HAS_AVX512F_SUPPORT)
  using simd_native = simd<arch::ISA::AVX512VLF, _Type_, 256>;

#elif defined(RAZE_HAS_AVX512BW_SUPPORT)
  using simd_native = simd<arch::ISA::AVX512BW, _Type_, 512>;

#elif defined(RAZE_HAS_AVX512DQ_SUPPORT)
  using simd_native = simd<arch::ISA::AVX512DQ, _Type_, 512>;

#elif defined(RAZE_HAS_AVX512F_SUPPORT)
  using simd_native = simd<arch::ISA::AVX512F, _Type_, 512>;

#elif defined(RAZE_HAS_AVX2_SUPPORT)
  using simd_native = simd<arch::ISA::AVX2, _Type_, 256>;

#elif defined(RAZE_HAS_SSE42_SUPPORT)
  using simd_native = simd<arch::ISA::SSE42, _Type_, 128>;

#elif defined(RAZE_HAS_SSE41_SUPPORT)
  using simd_native = simd<arch::ISA::SSE41, _Type_, 128>;

#elif defined(RAZE_HAS_SSSE3_SUPPORT)
  using simd_native = simd<arch::ISA::SSSE3, _Type_, 128>;
  
#elif defined(RAZE_HAS_SSE3_SUPPORT)
  using simd_native = simd<arch::ISA::SSE3, _Type_, 128>;

#elif defined(RAZE_HAS_SSE2_SUPPORT)
  using simd_native = simd<arch::ISA::SSE2, _Type_, 128>;

#else 
  #error "Unsupported architecture" 

#endif


#if defined(RAZE_FORCE_AVX512VBMI2)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512VBMI2, _Type_, 512>;

#elif defined(RAZE_FORCE_AVX512VBMI)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512VBMI, _Type_, 512>;

#elif defined(RAZE_FORCE_AVX512DQ) && defined(RAZE_FORCE_AVX512BW)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512BWDQ, _Type_, 512>;

#elif defined(RAZE_FORCE_AVX512VL) && defined(RAZE_FORCE_AVX512BW) && defined(RAZE_FORCE_AVX512DQ)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512VLBWDQ, _Type_, 256>;

#elif defined(RAZE_FORCE_AVX512VL) && defined(RAZE_FORCE_AVX512BW)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512VLBW, _Type_, 256>;

#elif defined(RAZE_FORCE_AVX512VL) && defined(RAZE_FORCE_AVX512DQ)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512VLDQ, _Type_, 256>;

#elif defined(RAZE_FORCE_AVX512VL) && defined(RAZE_FORCE_AVX512F)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512VLF, _Type_, 256>;

#elif defined(RAZE_FORCE_AVX512BW)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512BW, _Type_, 512>;

#elif defined(RAZE_FORCE_AVX512DQ)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512DQ, _Type_, 512>;

#elif defined(RAZE_FORCE_AVX512F)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX512F, _Type_, 512>;

#elif defined(RAZE_FORCE_AVX2)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::AVX2, _Type_, 256>;

#elif defined(RAZE_FORCE_SSE42)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::SSE42, _Type_, 128>;

#elif defined(RAZE_FORCE_SSE41)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::SSE41, _Type_, 128>;

#elif defined(RAZE_FORCE_SSSE3)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::SSSE3, _Type_, 128>;
  
#elif defined(RAZE_FORCE_SSE3)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::SSE3, _Type_, 128>;

#elif defined(RAZE_FORCE_SSE2)
  template <typename _Type_>
  using simd_forced = simd<arch::ISA::SSE2, _Type_, 128>;

#endif


__RAZE_DATAPAR_NAMESPACE_END
