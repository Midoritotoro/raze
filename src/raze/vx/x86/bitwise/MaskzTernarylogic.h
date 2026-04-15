#pragma once 

#include <src/raze/vx/bitwise/Ternarylogic.h>
#include <src/raze/vx/arithmetic/MaskzAssign.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    arch::ISA   _ISA_,
    uint32      _Width_,
    class       _Type_>
struct _Maskz_ternarylogic {
    template <
        class _IntrinType_,
        class _MaskType_,  
        uint8 _TernaryMask_>
    raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
        _IntrinType_                                    __x,
        _IntrinType_                                    __y,
        _IntrinType_                                    __z,
        std::integral_constant<uint8, _TernaryMask_>    __imm8,
        _MaskType_                                      __mask) raze_const_operator noexcept
    {
        if constexpr (__has_avx512f_support_v<_ISA_> && std::is_integral_v<_MaskType_>) {
            if constexpr (_Width_ == 512) {
                if constexpr (sizeof(_Type_) == 8)
                    return __as<_IntrinType_>(_mm512_maskz_ternarylogic_epi64(__mask, __as<__m512i>(__x), 
                        __as<__m512i>(__y), __as<__m512i>(__z), __imm8));
                else if constexpr (sizeof(_Type_) == 4)
                    return __as<_IntrinType_>(_mm512_maskz_ternarylogic_epi32(__mask, __as<__m512i>(__x),
                        __as<__m512i>(__y), __as<__m512i>(__z), __imm8));
            }
            else if constexpr (__has_avx512vl_support_v<_ISA_> && (_Width_ == 256)) {
                if constexpr (sizeof(_Type_) == 8)
                    return __as<_IntrinType_>(_mm256_maskz_ternarylogic_epi64(__mask, __as<__m256i>(__x),
                        __as<__m256i>(__y), __as<__m256i>(__z), __imm8));
                else if constexpr (sizeof(_Type_) == 4)
                    return __as<_IntrinType_>(_mm256_maskz_ternarylogic_epi32(__mask, __as<__m256i>(__x),
                        __as<__m256i>(__y), __as<__m256i>(__z), __imm8));
            }
            else if constexpr (__has_avx512vl_support_v<_ISA_> && (_Width_ == 128)) {
                if constexpr (sizeof(_Type_) == 8)
                    return __as<_IntrinType_>(_mm_maskz_ternarylogic_epi64(__mask, __as<__m128i>(__x),
                        __as<__m128i>(__y), __as<__m128i>(__z), __imm8));
                else if constexpr (sizeof(_Type_) == 4)
                    return __as<_IntrinType_>(_mm_maskz_ternarylogic_epi32(__mask, __as<__m128i>(__x),
                        __as<__m128i>(__y), __as<__m128i>(__z), __imm8));
            }
        }
       
        return _Maskz_assign<_ISA_, _Width_, _Type_>()(
            _Ternarylogic<_ISA_, _Width_>()(__x, __y, __z, __imm8), __mask);
    }
};

__RAZE_VX_NAMESPACE_END
