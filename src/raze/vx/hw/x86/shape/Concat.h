#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/vx/storage/VectorStorage.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, class V>
concept can_concatenate_into_native_register = (vector_default_size<ISA> / 8) >= (sizeof(V) * 2);

template <arch::ISA ISA, intrin_type V>
raze_always_inline auto concat_(V x, V y) noexcept {
	if constexpr (can_concatenate_into_native_register<ISA, V>) {
		if constexpr (sizeof(V) == 16 && has_avx<ISA>) return _mm256_insertf128_si256(as<__m256i>(x), as<__m128i>(y), 1);
		else if constexpr (sizeof(V) == 32 && has_avx512f<ISA>) return _mm512_inserti32x8(as<__m512i>(x), as<__m256i>(y), 1);
	}
}

__RAZE_VX_NAMESPACE_END
