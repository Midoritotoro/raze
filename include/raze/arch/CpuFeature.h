#pragma once 

#include <raze/Types.h>


__RAZE_ARCH_NAMESPACE_BEGIN

enum class ISA : raze::uchar {
	None,
	SSE,
	SSE2,
	SSE3,
	SSSE3,
	SSE41,
	SSE42,
	AVX,
	AVX2,
	AVX512F,
	AVX512BW,
	AVX512DQ,
	AVX512BWDQ,			// AVX512BW + AVX512DQ
	AVX512VLBWDQ,		// AVX512VL + AVX512BW + AVX512DQ
	AVX512VLDQ,			// AVX512VL + AVX512DQ
	AVX512VLBW,			// AVX512VL + AVX512BW
	AVX512VLF,			// AVX512VL + AVX512F
	AVX512VBMI,			// AVX512VBMI + AVX512BW
	AVX512VBMI2,		// AVX512VBMI2 + AVX512BW
	AVX512VBMIVL,		// AVX512VBMI + AVX512VL
	AVX512VBMI2VL,		// AVX512VBMI2 + AVX512VL
	AVX512VBMIDQ,		// AVX512VBMI + AVX512BW + AVX512DQ
	AVX512VBMI2DQ,		// AVX512VBMI2 + AVX512BW + AVX512DQ
	AVX512VBMIVLDQ,		// AVX512VBMI + AVX512BW + AVX512DQ + AVX512VL
	AVX512VBMI2VLDQ,	// AVX512VBMI2 + AVX512BW + AVX512DQ + AVX512VL
};

template <
	ISA	_Feature_,
	ISA	Candidate,
	typename	Enable = void>
struct __is_in_list_helper:
	std::false_type
{};

template <
	ISA _Feature_,
	ISA _Candidate_>
struct __is_in_list_helper<
	_Feature_, _Candidate_,
	std::enable_if_t<(_Feature_ == _Candidate_)>>:
		std::true_type
{};

template <
	ISA		_Feature_,
	ISA ...	_List_>
struct __contains {
	static constexpr bool value = (__is_in_list_helper<_Feature_, _List_>::value || ...);
};

#define __xmm_features arch::ISA::SSE, arch::ISA::SSE2, arch::ISA::SSE3, arch::ISA::SSSE3, arch::ISA::SSE41, arch::ISA::SSE42
#define __ymm_features arch::ISA::AVX, arch::ISA::AVX2
#define __zmm_features arch::ISA::AVX512F, arch::ISA::AVX512BW, arch::ISA::AVX512BWDQ, \
	arch::ISA::AVX512DQ, arch::ISA::AVX512VLDQ, arch::ISA::AVX512VLBW, arch::ISA::AVX512VLF, \
	arch::ISA::AVX512VLBWDQ, arch::ISA::AVX512VBMI, arch::ISA::AVX512VBMI2, arch::ISA::AVX512VBMIDQ,\
	arch::ISA::AVX512VBMI2DQ, arch::ISA::AVX512VBMIVL, arch::ISA::AVX512VBMI2VL, arch::ISA::AVX512VBMIVLDQ, arch::ISA::AVX512VBMI2VLDQ

template <arch::ISA _SimdGeneration_> 
constexpr inline bool __is_xmm_v = __contains<_SimdGeneration_, __xmm_features>::value;

template <arch::ISA _SimdGeneration_>
constexpr inline bool __is_ymm_v = __contains<_SimdGeneration_, __ymm_features>::value;

template <arch::ISA _SimdGeneration_>
constexpr inline bool __is_zmm_v = __contains<_SimdGeneration_, __zmm_features>::value;

__RAZE_ARCH_NAMESPACE_END
