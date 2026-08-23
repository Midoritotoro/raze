#pragma once 

#include <raze/Types.h>


__RAZE_ARCH_NAMESPACE_BEGIN

enum class ISA : raze::uchar {
	None, SSE, SSE2, SSE3, SSSE3, SSE41, SSE42,
	AVX, FMA3, AVX2,
	AVX2FMA3,			// AXV2 + FMA3
	AVX512F, AVX512BW, AVX512DQ,
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

template <ISA Feature, ISA Candidate, class	Enable = void>
struct is_in_list_helper:
	std::false_type
{};

template <ISA Feature, ISA Candidate>
struct is_in_list_helper<Feature, Candidate, std::enable_if_t<(Feature == Candidate)>>:
	std::true_type
{};

template <ISA Feature, ISA ... List>
struct contains {
	static constexpr bool value = (is_in_list_helper<Feature, List>::value || ...);
};

#define __xmm_features arch::ISA::SSE, arch::ISA::SSE2, arch::ISA::SSE3, arch::ISA::SSSE3, arch::ISA::SSE41, arch::ISA::SSE42
#define __ymm_features arch::ISA::AVX, arch::ISA::AVX2, arch::ISA::AVX2FMA3, arch::ISA::FMA3
#define __zmm_features arch::ISA::AVX512F, arch::ISA::AVX512BW, arch::ISA::AVX512BWDQ, \
	arch::ISA::AVX512DQ, arch::ISA::AVX512VLDQ, arch::ISA::AVX512VLBW, arch::ISA::AVX512VLF, \
	arch::ISA::AVX512VLBWDQ, arch::ISA::AVX512VBMI, arch::ISA::AVX512VBMI2, arch::ISA::AVX512VBMIDQ,\
	arch::ISA::AVX512VBMI2DQ, arch::ISA::AVX512VBMIVL, arch::ISA::AVX512VBMI2VL, arch::ISA::AVX512VBMIVLDQ, arch::ISA::AVX512VBMI2VLDQ

template <arch::ISA ISA> 
constexpr inline bool is_xmm_v = contains<ISA, __xmm_features>::value;

template <arch::ISA ISA>
constexpr inline bool is_ymm_v = contains<ISA, __ymm_features>::value;

template <arch::ISA ISA>
constexpr inline bool is_zmm_v = contains<ISA, __zmm_features>::value;

__RAZE_ARCH_NAMESPACE_END
