#pragma once 

#include <raze/vx/BasicSimd.h>
#include <raze/vx/SimdConfig.h>


__RAZE_VX_NAMESPACE_BEGIN

template <typename _Type_>
using simd128_sse2			= simd<_Type_, x86_runtime_abi<arch::ISA::SSE2, 128>>;

template <typename _Type_>
using simd128_sse3			= simd<_Type_, x86_runtime_abi<arch::ISA::SSE3, 128>>;

template <typename _Type_>
using simd128_ssse3			= simd<_Type_, x86_runtime_abi<arch::ISA::SSSE3, 128>>;

template <typename _Type_>
using simd128_sse41			= simd<_Type_, x86_runtime_abi<arch::ISA::SSE41, 128>>;

template <typename _Type_>
using simd128_sse42			= simd<_Type_, x86_runtime_abi<arch::ISA::SSE42, 128>>;

template <typename _Type_>
using simd128_avx512vlbw	= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLBW, 128>>;

template <typename _Type_>
using simd128_avx512vlf		= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLF, 128>>;

template <typename _Type_>
using simd128_avx512vldq	= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLDQ, 128>>;


template <typename _Type_>
using simd256_avx			= simd<_Type_, x86_runtime_abi<arch::ISA::AVX, 256>>;

template <typename _Type_>
using simd256_avx2			= simd<_Type_, x86_runtime_abi<arch::ISA::AVX2, 256>>;

template <typename _Type_>
using simd256_avx512vlbw	= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLBW, 256>>;

template <typename _Type_>
using simd256_avx512vlf		= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLF, 256>>;

template <typename _Type_>
using simd256_avx512vldq	= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLDQ, 256>>;

template <typename _Type_>
using simd256_avx512vlbwdq	= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLBWDQ, 256>>;


template <typename _Type_>
using simd512_avx512f		= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512F, 512>>;

template <typename _Type_>
using simd512_avx512bw		= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512BW, 512>>;

template <typename _Type_>
using simd512_avx512dq		= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512DQ, 512>>;

template <typename _Type_>
using simd512_avx512bwdq	= simd<_Type_, x86_runtime_abi<arch::ISA::AVX512BWDQ, 512>>;


template <typename _Type_>
#if defined(RAZE_HAS_AVX512VBMI2_SUPPORT) 
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VBMI2, 512>>;

#elif defined(RAZE_HAS_AVX512VBMI_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VBMI, 512>>;

#elif defined(RAZE_HAS_AVX512DQ_SUPPORT) && defined(RAZE_HAS_AVX512BW_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512BWDQ, 512>>;

#elif defined(RAZE_HAS_AVX512VL_SUPPORT) && defined(RAZE_HAS_AVX512BW_SUPPORT) && defined(RAZE_HAS_AVX512DQ_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLBWDQ, 256>>;

#elif defined(RAZE_HAS_AVX512VL_SUPPORT) && defined(RAZE_HAS_AVX512BW_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLBW, 256>>;

#elif defined(RAZE_HAS_AVX512VL_SUPPORT) && defined(RAZE_HAS_AVX512DQ_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLDQ, 256>>;

#elif defined(RAZE_HAS_AVX512VL_SUPPORT) && defined(RAZE_HAS_AVX512F_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLF, 256>>;

#elif defined(RAZE_HAS_AVX512BW_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512BW, 512>>;

#elif defined(RAZE_HAS_AVX512DQ_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512DQ, 512>>;

#elif defined(RAZE_HAS_AVX512F_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512F, 512>>;

#elif defined(RAZE_HAS_AVX2_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::AVX2, 256>>;

#elif defined(RAZE_HAS_SSE42_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::SSE42, 128>>;

#elif defined(RAZE_HAS_SSE41_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::SSE41, 128>>;

#elif defined(RAZE_HAS_SSSE3_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::SSSE3, 128>>;
  
#elif defined(RAZE_HAS_SSE3_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::SSE3, 128>>;

#elif defined(RAZE_HAS_SSE2_SUPPORT)
  using simd_native = simd<_Type_, x86_runtime_abi<arch::ISA::SSE2, 128>>;

#else 
  #error "Unsupported architecture" 

#endif


#if defined(RAZE_FORCE_AVX512VBMI2)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VBMI2, 512>>;

#elif defined(RAZE_FORCE_AVX512VBMI)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VBMI, 512>>;

#elif defined(RAZE_FORCE_AVX512DQ) && defined(RAZE_FORCE_AVX512BW)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512BWDQ, 512>>;

#elif defined(RAZE_FORCE_AVX512VL) && defined(RAZE_FORCE_AVX512BW) && defined(RAZE_FORCE_AVX512DQ)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLBWDQ, 256>>;

#elif defined(RAZE_FORCE_AVX512VL) && defined(RAZE_FORCE_AVX512BW)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLBW, 256>>;

#elif defined(RAZE_FORCE_AVX512VL) && defined(RAZE_FORCE_AVX512DQ)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLDQ, 256>>;

#elif defined(RAZE_FORCE_AVX512VL) && defined(RAZE_FORCE_AVX512F)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512VLF, 256>>;

#elif defined(RAZE_FORCE_AVX512BW)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512BW, 512>>;

#elif defined(RAZE_FORCE_AVX512DQ)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512DQ, 512>>;

#elif defined(RAZE_FORCE_AVX512F)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX512F, 512>>;

#elif defined(RAZE_FORCE_AVX2)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::AVX2, 256>>;

#elif defined(RAZE_FORCE_SSE42)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::SSE42, 128>>;

#elif defined(RAZE_FORCE_SSE41)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::SSE41, 128>>;

#elif defined(RAZE_FORCE_SSSE3)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::SSSE3, 128>>;
  
#elif defined(RAZE_FORCE_SSE3)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::SSE3, 128>>;

#elif defined(RAZE_FORCE_SSE2)
  template <typename _Type_>
  using simd_forced = simd<_Type_, x86_runtime_abi<arch::ISA::SSE2, 128>>;

#endif


__RAZE_VX_NAMESPACE_END
