#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype VectorBytes, sizetype ElementBytes, class IdxType>
consteval auto make_rotate_left_shuffle_table_() noexcept {
    constexpr sizetype num_elements = VectorBytes / ElementBytes;
    constexpr sizetype idx_bytes = sizeof(IdxType);
    constexpr sizetype num_indices = VectorBytes / idx_bytes;

    constexpr sizetype chunks_per_element = ElementBytes / idx_bytes;
    constexpr sizetype num_shifts = num_elements;

    std::array<std::array<IdxType, num_indices>, num_shifts> table{};

    for (sizetype shift = 0; shift < num_shifts; ++shift) {
        sizetype actual_shift = shift % num_elements;

        for (sizetype idx = 0; idx < num_indices; ++idx) {
            sizetype dst_elem = idx / chunks_per_element;
            sizetype chunk_offset = idx % chunks_per_element;
            sizetype src_elem = (dst_elem + actual_shift) % num_elements;
            sizetype src_chunk = src_elem * chunks_per_element + chunk_offset;

            table[shift][idx] = static_cast<IdxType>(src_chunk);
        }
    }
    return table;
}

template <arch::ISA ISA, arithmetic_type T, intrin_type V>
raze_always_inline auto make_rotate_left_idx_(V, i32 sh) noexcept {
    constexpr auto vector_bytes = sizeof(V);
    constexpr auto element_bytes = sizeof(T);
    using IdxType = typename IntegerForSizeof<T>::Unsigned;

    alignas(sizeof(V)) static constexpr auto table_u8 = make_rotate_left_shuffle_table_<vector_bytes, element_bytes, u8>();
    alignas(sizeof(V)) static constexpr auto table = make_rotate_left_shuffle_table_<vector_bytes, element_bytes, IdxType>();

    if constexpr (sizeof(V) == 16) return rotate_indices<V, u8> { load_<ISA, V>(table_u8[sh].data(), aligned_policy{}) };
    else if constexpr (vector_bytes == 32) {
        if constexpr (has_avx2<ISA> && !(has_avx512bw<ISA> && has_avx512vl<ISA>)) {
            if constexpr (sizeof(IdxType) >= 4) {
                alignas(sizeof(V)) static constexpr auto table_u32 = make_rotate_left_shuffle_table_<vector_bytes, element_bytes, u32>();
                return rotate_indices<V, u32>{ load_<ISA, V>(table_u32[sh].data(), aligned_policy{}) };
            }
            else return rotate_indices<V, u8>{ load_<ISA, V>(table_u8[sh].data(), aligned_policy{}) };
        }
        else return rotate_indices<V, IdxType>{ load_<ISA, V>(table[sh].data(), aligned_policy{}) };
    }
    else if constexpr (vector_bytes == 64) return rotate_indices<V, IdxType>{ load_<ISA, V>(table[sh].data(), aligned_policy{}) };
}

template <simd_type V, class Int>
raze_always_inline V rotate_left_fallback_(const V& x, Int sh) noexcept {
    alignas(sizeof(V)) typename V::value_type arr[V::size() * 2];

    vx::store[vx::aligned](arr, x);
    vx::store[vx::aligned](arr + V::size(), x);

    return vx::load<V>[vx::aligned](arr + sh);
}

template <class Pattern>
raze_always_inline pattern_vector_t<Pattern> rotate_left_(const pattern_vector_t<Pattern>& x, Pattern p) noexcept {
	using Ret = decltype(generic_shuffle_(x, p));
	if constexpr (is_fallback<Ret> || !native<pattern_vector_t<Pattern>>) return rotate_left_fallback_(x, get_rotate_left_shift(p));
	else return generic_shuffle_(x, p);
}

template <simd_type V>
raze_always_inline V rotate_left_(const V& x, i32 sh) noexcept {
	using Abi = abi_t<V>;
	using Value = typename V::value_type;

	if constexpr (native<V>) {
        using Intrin = decltype(ustorage(x.template __get<0>()));
        using RetRotate = decltype(make_rotate_left_idx_<Abi::isa, Value>(Intrin{}, sh));
        using IdxType = typename RetRotate::index_type;

        using Ret = decltype(generic_shuffle_native_<Abi::isa, IdxType>(Intrin{}, std::declval<RetRotate>().data()));
		
		if constexpr (is_fallback<Ret>) return rotate_left_fallback_(x, sh);
		else {
			auto r = x;
			auto& storage = r.template __get<0>();

            const auto rotate_indices = make_rotate_left_idx_<Abi::isa,
                Value>(ustorage(r.template __get<0>()), sh);

            using IndexType = typename decltype(rotate_indices)::index_type;
			storage = generic_shuffle_native_<Abi::isa, IndexType>(ustorage(storage), rotate_indices.data());
			
			return r;
		}
	}
	else {
		return rotate_left_fallback_(x, sh);
	}
}

__RAZE_VX_NAMESPACE_END
