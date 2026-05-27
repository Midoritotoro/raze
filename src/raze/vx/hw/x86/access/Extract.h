#pragma once 

#include <raze/memory/PointerToIntegral.h>
#include <src/raze/vx/hw/x86/memory/Load.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    arch::ISA	        _ISA_,
    arithmetic_type     _Element_>
struct _Extract {
    template <intrin_or_arithmetic_type _Tp_>
    raze_nodiscard raze_static_operator raze_always_inline _Element_ operator()(
        _Tp_    __vector,
        u8	__index) raze_const_operator noexcept
    {
        if constexpr (arithmetic_type<_Tp_>) {
            return __vector;
        }
        else {
            constexpr auto __length = sizeof(_Tp_) / sizeof(_Element_);

            alignas(sizeof(_Tp_)) _Element_ __array[__length];
            _Store<_ISA_>()(__array, __vector);

            return __array[__index & (__length - 1)];
        }
    }
};

template <intrin_type _ExtractVector_, arch::ISA _ISA_>
struct _Extract_vector {
	template <intrin_type _Tp_, sizetype _I_>
	raze_static_operator raze_always_inline _ExtractVector_ operator()(_Tp_ __vector, std::integral_constant<sizetype, _I_> __index) raze_const_operator noexcept {
		if constexpr (sizeof(_Tp_) == sizeof(_ExtractVector_)) {
			return __as<_Tp_>(__vector);
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (sizeof(_Tp_) == 16) {
				return _mm256_extracti128_si256(__as<__m256i>(__vector), __index);
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (sizeof(_Tp_) == 16) {
				__vector = _mm512_extracti64x2_epi64(__as<__m512i>(__vector), __index);
			}
			else if constexpr (sizeof(_Tp_) == 32) {
				__vector = _mm512_extracti64x4_epi64(__as<__m512i>(__vector), __index);
			}
		}
	}
};

__RAZE_VX_NAMESPACE_END
