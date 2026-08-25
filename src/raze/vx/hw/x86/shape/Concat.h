#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/vx/storage/VectorStorage.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, class _Tp_>
concept __can_concatenate_into_native_register = (__vector_default_size<_ISA_> / 8) >= (sizeof(_Tp_) * 2);

template <arch::ISA _ISA_>
struct _Concat {
	template <intrin_type _Tp_>
	raze_nodiscard raze_static_operator raze_always_inline auto operator()(_Tp_ __x, _Tp_ __y) raze_const_operator noexcept {
		if constexpr (__can_concatenate_into_native_register<_ISA_, _Tp_>) {
			if constexpr (sizeof(_Tp_) == 16 && has_avx<_ISA_>) return _mm256_insertf128_si256(as<__m256i>(__x), as<__m128i>(__y), 1);
			else if constexpr (sizeof(_Tp_) == 32 && has_avx512f<_ISA_>) return _mm512_inserti32x8(as<__m512i>(__x), as<__m256i>(__y), 1);
		}
	}
};

__RAZE_VX_NAMESPACE_END
