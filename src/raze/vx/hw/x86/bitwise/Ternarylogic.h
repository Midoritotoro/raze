#pragma once 

#include <src/raze/vx/hw/x86/bitwise/Andnot.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/bitwise/BitOr.h>
#include <src/raze/vx/hw/x86/bitwise/BitXor.h>
#include <src/raze/vx/hw/x86/bitwise/BitNot.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>
#include <src/raze/vx/hw/x86/bitwise/emulated/Ternarylogic.h>


__RAZE_VX_NAMESPACE_BEGIN



template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Ternarylogic {
    template <intrin_or_arithmetic_type _Tp_, u8 _TernaryMask_>
    raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(
        _Tp_ __x, _Tp_ __y,  _Tp_ __z, std::integral_constant<u8, _TernaryMask_> __imm8) raze_const_operator noexcept
    {
        if constexpr (__has_avx512f_support_v<_ISA_> && intrin_type<_Tp_>) {
            if constexpr (sizeof(_Tp_) == 64)  return __as<_Tp_>(_mm512_ternarylogic_epi32(__as<__m512i>(__x), __as<__m512i>(__y), __as<__m512i>(__z), __imm8));
            else if constexpr (__has_avx512vl_support_v<_ISA_> && sizeof(_Tp_) == 32) return __as<_Tp_>(_mm256_ternarylogic_epi32(__as<__m256i>(__x), __as<__m256i>(__y), __as<__m256i>(__z), __imm8));
            else if constexpr (__has_avx512vl_support_v<_ISA_> && sizeof(_Tp_) == 16) return __as<_Tp_>(_mm_ternarylogic_epi32(__as<__m128i>(__x), __as<__m128i>(__y),  __as<__m128i>(__z), __imm8));
        }

        return _Ternarylogic_emulated()(__x, __y, __z, __imm8, _Or<_ISA_, _Type_>(), _Xor<_ISA_, _Type_>(),
            _And<_ISA_, _Type_>(), _Andnot<_ISA_, _Type_>(), _Not<_ISA_, _Type_>(), _Zero<_ISA_, _Tp_>());
    }

    template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_, u8 _TernaryMask_>
    raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y,  _Tp_ __z, 
        std::integral_constant<u8, _TernaryMask_> __imm8,  _Mask_ __mask) const noexcept
    {
        if constexpr (__has_avx512f_support_v<_ISA_> && std::is_integral_v<_Mask_> && intrin_type<_Tp_>) {
            if constexpr (sizeof(_Tp_) == 64) {
                if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm512_maskz_ternarylogic_epi64(__mask, __as<__m512i>(__x), __as<__m512i>(__y), __as<__m512i>(__z), __imm8));
                else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm512_maskz_ternarylogic_epi32(__mask, __as<__m512i>(__x), __as<__m512i>(__y), __as<__m512i>(__z), __imm8));
            }
            else if constexpr (__has_avx512vl_support_v<_ISA_> && sizeof(_Tp_) == 32) {
                if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm256_maskz_ternarylogic_epi64(__mask, __as<__m256i>(__x),  __as<__m256i>(__y), __as<__m256i>(__z), __imm8));
                else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm256_maskz_ternarylogic_epi32(__mask, __as<__m256i>(__x), __as<__m256i>(__y), __as<__m256i>(__z), __imm8));
            }
            else if constexpr (__has_avx512vl_support_v<_ISA_> && sizeof(_Tp_) == 16) {
                if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm_maskz_ternarylogic_epi64(__mask, __as<__m128i>(__x), __as<__m128i>(__y), __as<__m128i>(__z), __imm8));
                else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm_maskz_ternarylogic_epi32(__mask, __as<__m128i>(__x), __as<__m128i>(__y), __as<__m128i>(__z), __imm8));
            }
        }
       
        return _Selectz<_ISA_, _Type_>()((*this)(__x, __y, __z, __imm8), __mask);
    }


    template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_, u8 _TernaryMask_>
    raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y,  _Tp_ __z, 
        std::integral_constant<u8, _TernaryMask_> __imm8,  _Mask_ __mask, _Tp_ __src) const noexcept
    {
        return _Select<_ISA_, _Type_>()((*this)(__x, __y, __z, __imm8), __src, __mask);
    }
};


__RAZE_VX_NAMESPACE_END
