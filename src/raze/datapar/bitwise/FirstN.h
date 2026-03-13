#pragma once 

#include <src/raze/datapar/bitwise/MaskConvert.h>
#include <src/raze/datapar/MaskTypeSelector.h>
#include <raze/algorithm/minmax/Min.h>
#include <array>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <sizetype _VectorLength_>
consteval auto __generate_first_n_table() noexcept {
    using _MaskType = __mmask_for_elements_t<_VectorLength_>;
    auto __table = std::array<_MaskType, _VectorLength_ + 1>{};

    for (auto __i = 0; __i <= _VectorLength_; ++__i) {
        if (__i == _VectorLength_) {
            if (_VectorLength_ >= 8)
                __table[__i] = math::__maximum_integral_limit<_MaskType>();
            else
                __table[__i] = _MaskType((_MaskType(1) << __i) - 1);
        }
        else {
            __table[__i] = _MaskType((_MaskType(1) << __i) - 1);
        }
    }

    return __table;
}

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_DesiredType_>
struct _First_n;

template <class _DesiredType_>
struct _First_n<arch::ISA::SSE2, 128, _DesiredType_> {
	raze_nodiscard raze_static_operator raze_always_inline 
		auto operator()(uint32 __elements) raze_const_operator noexcept
	{
		alignas(16) static constexpr uint32 __first_n_mask[8] =  { ~0u, ~0u, ~0u, ~0u, 0, 0, 0, 0 };
        const auto __bytes = __elements * sizeof(_DesiredType_);

		return _mm_load_si128(reinterpret_cast<const __m128i*>(
			reinterpret_cast<const uint8*>(__first_n_mask) + (16 - __bytes)));
	}
};


template <class _DesiredType_>
struct _First_n<arch::ISA::AVX2, 256, _DesiredType_> {
	raze_nodiscard raze_static_operator raze_always_inline
		auto operator()(uint32 __elements) raze_const_operator noexcept
	{
		alignas(32) static constexpr uint32 __first_n_mask[16] = 
			{ ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, 0, 0, 0, 0, 0, 0, 0, 0 };
        const auto __bytes = __elements * sizeof(_DesiredType_);

		return _mm256_load_si256(reinterpret_cast<const __m256i*>(
			reinterpret_cast<const uint8*>(__first_n_mask) + (32 - __bytes)));
	}
};

template <class _DesiredType_>
struct _First_n<arch::ISA::AVX512F, 512, _DesiredType_> {
    raze_nodiscard raze_static_operator raze_always_inline
        auto operator()(uint32 __elements) raze_const_operator noexcept
    {
        constexpr auto __length = sizeof(__m512i) / sizeof(_DesiredType_);

        if constexpr (sizeof(_DesiredType_) >= 4) {
            using _MaskType = __mmask_for_elements_t<__length>;
            constexpr auto __table = __generate_first_n_table<__length>();
            return __table[__elements];
        } 
        else {
            const auto __bytes = __elements * sizeof(_DesiredType_);

            alignas(64) static constexpr uint32 __first_n_mask[32] = {
                ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u,
                ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0
            };

            return _mm512_load_si512(reinterpret_cast<const uint8*>(__first_n_mask) + (64 - __bytes));
        }
    }
};

template <class _DesiredType_>
struct _First_n<arch::ISA::AVX512BW, 512, _DesiredType_> :
    _First_n<arch::ISA::AVX512F, 512, _DesiredType_>
{
    raze_nodiscard raze_static_operator raze_always_inline
        auto operator()(uint32 __elements) raze_const_operator noexcept
    {
        constexpr auto __length = sizeof(__m512i) / sizeof(_DesiredType_);
        using _MaskType = __mmask_for_elements_t<__length>;
        constexpr auto __table = __generate_first_n_table<__length>();
        return __table[__elements];
    }
};

template <class _DesiredType_>
struct _First_n<arch::ISA::AVX512VLF, 256, _DesiredType_> :
    _First_n<arch::ISA::AVX2, 256, _DesiredType_>
{
    raze_nodiscard raze_static_operator raze_always_inline
        auto operator()(uint32 __elements) raze_const_operator noexcept 
    {
        constexpr auto __length = sizeof(__m256i) / sizeof(_DesiredType_);

        if constexpr (sizeof(_DesiredType_) >= 4) {
            using _MaskType = __mmask_for_elements_t<__length>;
            constexpr auto __table = __generate_first_n_table<__length>();
            return __table[__elements];
        } 
        else {
            const auto __bytes = __elements * sizeof(_DesiredType_);

            alignas(32) static constexpr uint32 __first_n_mask[16] = {
                ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u,
                0, 0, 0, 0, 0, 0, 0, 0
            };

            return _mm256_load_si256(reinterpret_cast<const __m256i*>(
                reinterpret_cast<const uint8*>(__first_n_mask) + (32 - __bytes)));
        }
    }
};

template <class _DesiredType_>
struct _First_n<arch::ISA::AVX512VLBW, 256, _DesiredType_> :
    _First_n<arch::ISA::AVX512VLF, 256, _DesiredType_>
{
    raze_nodiscard raze_static_operator raze_always_inline
        auto operator()(uint32 __elements) raze_const_operator noexcept
    {
        constexpr auto __length = sizeof(__m256i) / sizeof(_DesiredType_);
        using _MaskType = __mmask_for_elements_t<__length>;
        constexpr auto __table = __generate_first_n_table<__length>();
        return __table[__elements];
    }
};

template <class _DesiredType_>
struct _First_n<arch::ISA::AVX512VLF, 128, _DesiredType_> :
    _First_n<arch::ISA::AVX2, 128, _DesiredType_>
{
    raze_nodiscard raze_static_operator raze_always_inline
        auto operator()(uint32 __elements) raze_const_operator noexcept
    {
        constexpr auto __length = sizeof(__m128i) / sizeof(_DesiredType_);

        if constexpr (sizeof(_DesiredType_) >= 4) {
            using _MaskType = __mmask_for_elements_t<__length>;
            constexpr auto __table = __generate_first_n_table<__length>();
            return __table[__elements];
        } else {
            const auto __bytes = __elements * sizeof(_DesiredType_);

            alignas(16) static constexpr uint32 __first_n_mask[8] = {
                ~0u, ~0u, ~0u, ~0u, 0, 0, 0, 0
            };

            return _mm_load_si128(reinterpret_cast<const __m128i*>(
                reinterpret_cast<const uint8*>(__first_n_mask) + (16 - __bytes)));
        }
    }
};

template <class _DesiredType_>
struct _First_n<arch::ISA::AVX512VLBW, 128, _DesiredType_> :
    _First_n<arch::ISA::AVX512VLF, 128, _DesiredType_>
{
    raze_nodiscard raze_static_operator raze_always_inline
        auto operator()(uint32 __elements) raze_const_operator noexcept 
    {
        constexpr auto __length = sizeof(__m128i) / sizeof(_DesiredType_);
        using _MaskType = __mmask_for_elements_t<__length>;
        constexpr auto __table = __generate_first_n_table<__length>();
        return __table[__elements];
    }
};

template <class _DesiredType_> struct _First_n<arch::ISA::SSE3, 128, _DesiredType_> : _First_n<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::SSSE3, 128, _DesiredType_> : _First_n<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::SSE41, 128, _DesiredType_> : _First_n<arch::ISA::SSSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::SSE42, 128, _DesiredType_> : _First_n<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX2, 128, _DesiredType_> : _First_n<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _First_n<arch::ISA::AVX512DQ, 512, _DesiredType_> : _First_n<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _First_n<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _First_n<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _First_n<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _First_n<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _First_n<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _First_n<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _First_n<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _First_n<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _First_n<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _First_n<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _First_n<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _First_n<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _First_n<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _First_n<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _First_n<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _First_n<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _First_n<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _First_n<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
