#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

#pragma strict_gs_check(off)

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype VectorBytes, sizetype ElementBytes, class IdxType_>
consteval auto make_slide_left_pshufb_table_() noexcept {
    constexpr sizetype num_elements = VectorBytes / ElementBytes;
    constexpr sizetype idx_bytes = sizeof(IdxType);
    constexpr sizetype num_indices = VectorBytes / idx_bytes;

    constexpr sizetype chunks_per_element = ElementBytes / idx_bytes;
    constexpr sizetype num_shifts = num_elements;

    std::array<std::array<IdxType_, num_indices>, num_shifts> table{};

    for (sizetype shift = 0; shift < num_shifts; ++shift) {
        for (sizetype idx = 0; idx < num_indices; ++idx) {
            sizetype dst_elem = idx / chunks_per_element;
            sizetype chunk_offset = idx % chunks_per_element;
            sizetype src_elem = dst_elem + shift;

            if (src_elem >= num_elements) {
                if constexpr (sizeof(IdxType) == 1) table[shift][idx] = static_cast<IdxType>(0x80);
                else table[shift][idx] = static_cast<IdxType>(-1);
            }
            else {
                sizetype src_chunk = src_elem * chunks_per_element + chunk_offset;
                table[shift][idx] = static_cast<IdxType>(src_chunk);
            }
        }
    }

    return table;
}

template <arch::ISA ISA, arithmetic_type T, intrin_type V>
raze_always_inline auto make_pshufb_slide_left_idx_(V, i32 sh) noexcept {
    using IdxType = typename IntegerForSizeof<T>::Unsigned;
    alignas(sizeof(V)) static constexpr auto table_u8 = make_slide_left_pshufb_table_<sizeof(V), sizeof(T), u8>();
    return rotate_indices<V, u8> { load_<ISA, V>(table_u8[_sh & (sizeof(V) - 1)].data(), aligned_policy{}) };
}

template <simd_type V, class Int>
raze_always_inline V slide_left_fallback_(const V& x, Int sh) noexcept {
    alignas(sizeof(V)) typename V::value_type arr[V::size() * 2];

    vx::store[vx::aligned](arr, x);
    vx::store[vx::aligned](arr + V::size(), V::zero());

    return vx::load<V>[vx::aligned](arr + sh);
}

template <intrin_type V, class Pattern>
raze_always_inline V slide_left_native_(V x, Pattern p) noexcept {
    constexpr auto isa = abi_t<pattern_vector_t<Pattern>>::isa;
    using Value = typename pattern_vector_t<Pattern>::value_type;

    constexpr auto shift = get_slide_left_shift(p) == shuffle_zero ? p.size() : get_slide_left_shift(p);
    constexpr auto shift_bytes = shift * sizeof(Value);
    constexpr auto size = p.size();

    if constexpr (sizeof(V) == 16) return as<V>(_mm_srli_si128(as<__m128i>(x), shift_bytes));
    else if constexpr (sizeof(V) == 32 && has_avx2<isa>) {
        if constexpr (has_avx512vl<isa> && (shift_bytes % 4) == 0) {
            return as<V>(_mm256_alignr_epi32(_mm256_setzero_si256(), as<__m256i>(x), (shift_bytes >> 2) & 7));
        }
        else {
            auto low_part = _mm256_setzero_si256();
            auto high_part = _mm256_setzero_si256();

            if constexpr (shift_bytes < 16) {
                low_part = _mm256_inserti128_si256(low_part, _mm256_extracti128_si256(as<__m256i>(x), 1), 0);
                high_part = as<__m256i>(x);
            }
            else if constexpr (shift_bytes < 32) high_part = _mm256_inserti128_si256(high_part, _mm256_extracti128_si256(as<__m256i>(x), 1), 0);
            else return zero_<isa, V>();

            if constexpr ((shift_bytes % 16) == 0) return as<V>(high_part);
            return as<V>(_mm256_alignr_epi8(low_part, high_part, shift_bytes & 0xF));
        }
    }
    else if constexpr (sizeof(V) == 64) {
        auto low_part = _mm512_setzero_si512();
        auto high_part = _mm512_setzero_si512();

        if constexpr (shift_bytes < 16) {
            low_part = _mm512_maskz_shuffle_i64x2(0x3F, as<__m512i>(x), as<__m512i>(x), 0x39);
            high_part = as<__m512i>(x);
        }
        else if constexpr (shift_bytes < 32) {
            low_part = _mm512_maskz_shuffle_i64x2(0x0F, as<__m512i>(x), as<__m512i>(x), 0x0E);
            high_part = _mm512_maskz_shuffle_i64x2(0x3F, as<__m512i>(x), as<__m512i>(x), 0x39);
        }
        else if constexpr (shift_bytes < 48) {
            low_part = _mm512_maskz_shuffle_i64x2(0x03, as<__m512i>(x), as<__m512i>(x), 0x03);
            high_part = _mm512_maskz_shuffle_i64x2(0x0F, as<__m512i>(x), as<__m512i>(x), 0x0E);
        }
        else if constexpr (shift_bytes < 64) {
            high_part = _mm512_maskz_shuffle_i64x2(0x03, as<__m512i>(x), as<__m512i>(x), 0x03);
        }
        else return zero_<isa, V>();

        if constexpr (has_avx512bw<isa>) {
            return as<V>(_mm512_alignr_epi8(low_part, high_part, shift_bytes & 0xF));
        }
        else {
            if constexpr ((shift_bytes % 4) == 0) return as<V>(_mm512_alignr_epi32(
                _mm512_setzero_si512(), as<__m512i>(x), (shift_bytes >> 2) & 0xF));

            const auto low256 = _mm256_alignr_epi8(as<__m256i>(low_part), as<__m256i>(high_part), shift_bytes & 0xF);
            const auto high256 = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(as<__m512i>(low_part), 1),
                _mm512_extracti64x4_epi64(as<__m512i>(high_part), 1), shift_bytes & 0xF);

            return as<V>(_mm512_inserti64x4(as<__m512i>(low256), high256, 1));
        }
    }
    else {
        alignas(sizeof(V)) Value arr[p.size() * 2];

        store_(arr, x, aligned_policy{});
        store_(arr + p.size(), zero_<isa, V>(), aligned_policy{});

        return store_<isa, V>(arr + shift, aligned_policy{});
    }
}

template <class Pattern>
raze_nodiscard raze_no_stack_protector raze_always_inline pattern_vector_t<Pattern> 
slide_left_(const pattern_vector_t<Pattern>& x, Pattern p) noexcept {
    using V = pattern_vector_t<Pattern>;

    if constexpr (get_slide_left_shift(p) == 0) return x;
    else if constexpr (get_slide_left_shift(p) >= V::size()) return V::zero();
    else if constexpr (native<V>) {
        auto r = x;

        auto& storage = r.template __get<0>();
        storage = slide_left_native_(ustorage(r.template __get<0>()), p);

        return r;
    }
    else return slide_left_fallback_(x, get_slide_left_shift(p) == shuffle_zero ? p.size() : get_slide_left_shift(p));
}

template <simd_type V>
raze_always_inline V slide_left_(const V& x, i32 __sh) noexcept {
	using Abi = abi_t<V>;
	using Value = typename V::value_type;

    static constexpr auto select_isa = []() constexpr noexcept {
        if constexpr (sizeof(V) == 16 && has_ssse3<Abi::isa>) return arch::ISA::SSSE3;
        else if constexpr (sizeof(V) == 32 && has_avx2<Abi::isa>) return arch::ISA::AVX2;
    };

	if constexpr (native<V> && ((sizeof(V) == 16 && has_ssse3<Abi::isa>)
        || (sizeof(V) == 32 && has_avx2<Abi::isa>)))
    {
        constexpr auto isa = select_isa();

        using Intrin_ = decltype(ustorage(x.template __get<0>()));
        using RetRotate = decltype(make_pshufb_slide_left_idx_<isa, Value>(Intrin{}, sh));
        using IdxType = typename RetRotate::index_type;

        using Ret = decltype(__generic_shuffle_native<isa, IdxType>(Intrin{}, std::declval<RetRotate>().data()));
		
		if constexpr (is_fallback<Ret>) return slide_left_fallback_(x, sh);
		else {
			auto r = x;
			auto& storage = r.template __get<0>();

            const auto rotate_indices = make_pshufb_slide_left_idx_<isa, Value>(ustorage(r.template __get<0>()), sh);
            using IndexType = typename decltype(rotate_indices)::index_type;

			storage = generic_shuffle_native_<isa, IndexType>(ustorage(storage), rotate_indices.data());
			
			return r;
		}
	}
	else return slide_left_fallback_(x, sh);
}

__RAZE_VX_NAMESPACE_END

#pragma strict_gs_check(on)