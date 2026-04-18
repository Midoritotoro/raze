#pragma once 

#include <raze/memory/PointerToIntegral.h>
#include <raze/math/IntegralTypesConversions.h>

#include <src/raze/vx/hw/x86/mask/MaskTypeSelector.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _To_mask;

template <class _Type_>
struct _To_mask<arch::ISA::SSE2, 128, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline 
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        using _MaskType = __mmask_for_elements_t<0x10 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;

        else if constexpr (sizeof(_Type_) == 8)
            return static_cast<_MaskType>(_mm_movemask_pd(__as<__m128d>(__vector)));

        else if constexpr (sizeof(_Type_) == 4)
            return static_cast<_MaskType>(_mm_movemask_ps(__as<__m128>(__vector)));

        else if constexpr (sizeof(_Type_) == 2)
            return static_cast<_MaskType>(_mm_movemask_epi8(_mm_packs_epi16(__as<__m128i>(__vector), _mm_setzero_si128())));

        else if constexpr (sizeof(_Type_) == 1)
            return static_cast<_MaskType>(_mm_movemask_epi8(__as<__m128i>(__vector)));
	}
};

template <class _Type_> 
struct _To_mask<arch::ISA::AVX, 256, _Type_> {
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        using _MaskType = __mmask_for_elements_t<0x20 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>) {
            return __vector;
        }
        else if constexpr (sizeof(_Type_) == 8) {
            return static_cast<_MaskType>(_mm256_movemask_pd(__as<__m256d>(__vector)));
        }
        else if constexpr (sizeof(_Type_) == 4) {
            return static_cast<_MaskType>(_mm256_movemask_ps(__as<__m256>(__vector)));
        }
        else {
            constexpr auto __xmm_bits = (sizeof(_IntrinType_) / sizeof(_Type_)) >> 1;

            const auto __low = _To_mask<arch::ISA::SSE42, 128, _Type_>()(
                __as<__m256i>(__vector));
            const auto __high = _To_mask<arch::ISA::SSE42, 128, _Type_>()(
                _mm256_extractf128_si256(__as<__m256i>(__vector), 1));

            return ((static_cast<_MaskType>(__high) << __xmm_bits) | static_cast<_MaskType>(__low));
        }
    }
};

template <class _Type_>
struct _To_mask<arch::ISA::AVX2, 256, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline 
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        using _MaskType = __mmask_for_elements_t<0x20 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>) {
            return __vector;
        }
        else if constexpr (sizeof(_Type_) == 8) {
            return static_cast<_MaskType>(_mm256_movemask_pd(__as<__m256d>(__vector)));
        }
        else if constexpr (sizeof(_Type_) == 4) {
            return static_cast<_MaskType>(_mm256_movemask_ps(__as<__m256>(__vector)));
        }
        else if constexpr (sizeof(_Type_) == 2) {
            const auto __pack = _mm256_packs_epi16(__as<__m256i>(__vector), _mm256_setzero_si256());
            const auto __shuffled = _mm256_permute4x64_epi64(__pack, 0xD8);

            return static_cast<_MaskType>(_mm256_movemask_epi8(__shuffled));
        }
        else if constexpr (sizeof(_Type_) == 1) {
            return static_cast<_MaskType>(_mm256_movemask_epi8(__as<__m256i>(__vector)));
        }
	}
};

template <class _Type_>
struct _To_mask<arch::ISA::AVX512F, 512, _Type_> {
	template <class _IntrinType_>
	raze_nodiscard raze_always_inline 
        auto operator()(_IntrinType_ __vector) const noexcept 
    {
        using _MaskType = __mmask_for_elements_t<0x40 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>) {
            return __vector;
        }
        else if constexpr (sizeof(_Type_) == 8) {
            return static_cast<_MaskType>(_mm512_cmp_epi64_mask(
                __as<__m512i>(__vector), _mm512_setzero_si512(), _MM_CMPINT_LT));
        }
        else if constexpr (sizeof(_Type_) == 4) {
            return static_cast<_MaskType>(_mm512_cmp_epi32_mask(
                __as<__m512i>(__vector), _mm512_setzero_si512(), _MM_CMPINT_LT));
        }
        else {
            constexpr auto __ymm_bits = (sizeof(_IntrinType_) / sizeof(_Type_)) >> 1;

            const auto __low = _To_mask<arch::ISA::AVX2, 256, _Type_>()(__as<__m256i>(__vector));
            const auto __high = _To_mask<arch::ISA::AVX2, 256, _Type_>()(_mm512_extractf64x4_pd(__as<__m512d>(__vector), 1));

            return ((static_cast<_MaskType>(__high) << __ymm_bits) | static_cast<_MaskType>(__low));
        }
	}
};

template <class _Type_> 
struct _To_mask<arch::ISA::AVX512BW, 512, _Type_>:
    _To_mask<arch::ISA::AVX512F, 512, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline 
        auto operator()(_IntrinType_ __vector) const noexcept 
    {
        using _MaskType = __mmask_for_elements_t<0x40 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;

        else if constexpr (sizeof(_Type_) == 2)
            return static_cast<_MaskType>(_mm512_movepi16_mask(__as<__m512i>(__vector)));

        else if constexpr (sizeof(_Type_) == 1)
            return static_cast<_MaskType>(_mm512_movepi8_mask(__as<__m512i>(__vector)));

        else
            return _To_mask<arch::ISA::AVX512F, 512, _Type_>()(__vector);
    }
};

template <class _Type_>
struct _To_mask<arch::ISA::AVX512BWDQ, 512, _Type_>: 
    _To_mask<arch::ISA::AVX512BW, 512, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        using _MaskType = __mmask_for_elements_t<0x40 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;

        else if constexpr (sizeof(_Type_) == 1)
            return static_cast<_MaskType>(_mm512_movepi8_mask(__as<__m512i>(__vector)));

        else if constexpr (sizeof(_Type_) == 2)
            return static_cast<_MaskType>(_mm512_movepi16_mask(__as<__m512i>(__vector)));

        else if constexpr (sizeof(_Type_) == 4)
            return static_cast<_MaskType>(_mm512_movepi32_mask(__as<__m512i>(__vector)));

        else if constexpr (sizeof(_Type_) == 8)
            return static_cast<_MaskType>(_mm512_movepi64_mask(__as<__m512i>(__vector)));
    }
};

template <class _Type_>
struct _To_mask<arch::ISA::AVX512DQ, 512, _Type_>:
    _To_mask<arch::ISA::AVX512F, 512, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        using _MaskType = __mmask_for_elements_t<0x40 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;

        else if constexpr (sizeof(_Type_) == 4)
            return static_cast<_MaskType>(_mm512_movepi32_mask(__as<__m512i>(__vector)));

        else if constexpr (sizeof(_Type_) == 8)
            return static_cast<_MaskType>(_mm512_movepi64_mask(__as<__m512i>(__vector)));

        else
            return _To_mask<arch::ISA::AVX512F, 512, _Type_>()(__vector);
    }
};

template <class _Type_> 
struct _To_mask<arch::ISA::AVX512VLBW, 256, _Type_>:
    _To_mask<arch::ISA::AVX512VLF, 256, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        using _MaskType = __mmask_for_elements_t<0x20 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;

        else if constexpr (sizeof(_Type_) == 1)
            return static_cast<_MaskType>(_mm256_movepi8_mask(__as<__m256i>(__vector)));

        else if constexpr (sizeof(_Type_) == 2)
            return static_cast<_MaskType>(_mm256_movepi16_mask(__as<__m256i>(__vector)));

        else
            return _To_mask<arch::ISA::AVX2, 256, _Type_>()(__vector);
    }
};

template <class _Type_> 
struct _To_mask<arch::ISA::AVX512VLDQ, 256, _Type_>: 
    _To_mask<arch::ISA::AVX512VLF, 256, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        using _MaskType = __mmask_for_elements_t<0x20 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;

        else if constexpr (sizeof(_Type_) == 4)
            return static_cast<_MaskType>(_mm256_movepi32_mask(__as<__m256i>(__vector)));

        else if constexpr (sizeof(_Type_) == 8)
            return static_cast<_MaskType>(_mm256_movepi64_mask(__as<__m256i>(__vector)));

        else
            return _To_mask<arch::ISA::AVX2, 256, _Type_>()(__vector);
    }
};

template <class _Type_> 
struct _To_mask<arch::ISA::AVX512VLBW, 128, _Type_>:
    _To_mask<arch::ISA::AVX512VLF, 128, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        using _MaskType = __mmask_for_elements_t<0x10 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;

        else if constexpr (sizeof(_Type_) == 1)
            return static_cast<_MaskType>(_mm_movepi8_mask(__as<__m128i>(__vector)));

        else if constexpr (sizeof(_Type_) == 8)
            return static_cast<_MaskType>(_mm_movepi16_mask(__as<__m128i>(__vector)));

        else
            return _To_mask<arch::ISA::SSE42, 128, _Type_>()(__vector);
    }
};

template <class _Type_> 
struct _To_mask<arch::ISA::AVX512VLDQ, 128, _Type_>:
    _To_mask<arch::ISA::AVX512VLF, 128, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        using _MaskType = __mmask_for_elements_t<0x10 / sizeof(_Type_)>;

        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;

        else if constexpr (sizeof(_Type_) == 4)
            return static_cast<_MaskType>(_mm_movepi32_mask(__as<__m128i>(__vector)));

        else if constexpr (sizeof(_Type_) == 8)
            return static_cast<_MaskType>(_mm_movepi64_mask(__as<__m128i>(__vector)));

        else
            return _To_mask<arch::ISA::SSE42, 128, _Type_>()(__vector);
    }
};

template <class _Type_> 
struct _To_mask<arch::ISA::AVX512VLBWDQ, 128, _Type_>:
    _To_mask<arch::ISA::AVX512VLBW, 128, _Type_> 
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;
        else if constexpr (sizeof(_Type_) <= 2)
            return _To_mask<arch::ISA::AVX512VLBW, 128, _Type_>()(__vector);
        else
            return _To_mask<arch::ISA::AVX512VLDQ, 128, _Type_>()(__vector);
    }
};

template <class _Type_>
struct _To_mask<arch::ISA::AVX512VLBWDQ, 256, _Type_> :
    _To_mask<arch::ISA::AVX512VLBW, 256, _Type_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_always_inline
        auto operator()(_IntrinType_ __vector) const noexcept
    {
        if constexpr (std::is_integral_v<_IntrinType_>)
            return __vector;
        else if constexpr (sizeof(_Type_) <= 2)
            return _To_mask<arch::ISA::AVX512VLBW, 256, _Type_>()(__vector);
        else
            return _To_mask<arch::ISA::AVX512VLDQ, 256, _Type_>()(__vector);
    }
};

template <class _Type_> struct _To_mask<arch::ISA::SSE3, 128, _Type_> : _To_mask<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::SSSE3, 128, _Type_> : _To_mask<arch::ISA::SSE3, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::SSE41, 128, _Type_> : _To_mask<arch::ISA::SSSE3, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::SSE42, 128, _Type_> : _To_mask<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX, 128, _Type_> : _To_mask<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX2, 128, _Type_> : _To_mask<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::FMA3, 128, _Type_> : _To_mask<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX2FMA3, 128, _Type_> : _To_mask<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMI, 512, _Type_> : _To_mask<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMI2, 512, _Type_> : _To_mask<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _To_mask<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _To_mask<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _To_mask<arch::ISA::FMA3, 256, _Type_> : _To_mask<arch::ISA::AVX, 256, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX2FMA3, 256, _Type_> : _To_mask<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VLF, 256, _Type_> : _To_mask<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMIVL, 256, _Type_> : _To_mask<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _To_mask<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _To_mask<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _To_mask<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _To_mask<arch::ISA::AVX512VLF, 128, _Type_> : _To_mask<arch::ISA::AVX2, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMIVL, 128, _Type_> : _To_mask<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _To_mask<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _To_mask<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _To_mask<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _To_mask<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
