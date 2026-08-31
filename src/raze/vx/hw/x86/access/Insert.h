#pragma once 

#include <src/raze/vx/hw/x86/construct/Broadcast.h>

#include <raze/math/BitCast.h>
#include <src/raze/vx/hw/x86/access/InsertMask.h>

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, intrin_or_arithmetic_type V, arithmetic_type T>
raze_always_inline void insert_(V& v, u8 i, T x) noexcept {
	if constexpr (arithmetic_type<V>) {
		v = x;
	}
	else {
		constexpr auto mask = make_insert_mask<V, typename IntegerForSizeof<T>::Unsigned>();

		const auto broadcasted = broadcast_<ISA, V>(x);
		const auto insert_mask = load_<ISA, V>(mask.array + mask.offset - i);

		v = select_<ISA, T>(broadcasted, v, insert_mask);
	}
}

template <arch::ISA ISA, intrin_type V, sizetype I, intrin_type Insert>
raze_always_inline void insert_vector_(V& v, std::integral_constant<sizetype, I> i, Insert ins) noexcept {
	if constexpr (sizeof(V) == sizeof(Insert)) v = as<V>(ins);
	else if constexpr (sizeof(V) == 32) {
		if constexpr (sizeof(Insert) == 16) v = _mm256_inserti128_si256(as<__m256i>(v), as<__m128i>(ins), i);
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (sizeof(Insert) == 16) v = _mm512_inserti32x4(as<__m512i>(v), as<__m128i>(ins), i);
		else if constexpr (sizeof(Insert) == 32) v = _mm512_inserti32x8(as<__m512i>(v), as<__m256i>(ins), i);
	}
}

__RAZE_VX_NAMESPACE_END
