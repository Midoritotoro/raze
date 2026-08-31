#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype VectorBytes, sizetype ElementBytes, class IdxType>
consteval auto make_slide_right_pshufb_table_() noexcept {
    constexpr sizetype num_elements = VectorBytes / ElementBytes;
    constexpr sizetype idx_bytes = sizeof(IdxType);
    constexpr sizetype num_indices = VectorBytes / idx_bytes;

    constexpr sizetype chunks_per_element = ElementBytes / idx_bytes;
    constexpr sizetype num_shifts = num_elements;

    std::array<std::array<IdxType, num_indices>, num_shifts> table{};

    for (sizetype shift = 0; shift < num_shifts; ++shift) {
        for (sizetype idx = 0; idx < num_indices; ++idx) {
            sizetype dst_elem = idx / chunks_per_element;
            sizetype chunk_offset = idx % chunks_per_element;

            if (dst_elem < shift) table[shift][idx] = static_cast<IdxType>(-1);
            else {
                sizetype src_elem = dst_elem - shift;
                sizetype src_chunk = src_elem * chunks_per_element + chunk_offset;

                table[shift][idx] = static_cast<IdxType>(src_chunk);
            }
        }
    }

    return table;
}

template <arch::ISA ISA, arithmetic_type T, intrin_type V>
raze_nodiscard raze_always_inline auto make_pshufb_slide_right_idx_(V, i32 sh) noexcept {
    alignas(sizeof(V)) static constexpr auto table_u8 = make_slide_right_pshufb_table_<sizeof(V), sizeof(T), u8>();
    return rotate_indices<V, u8>{ load_<ISA, V>(table_u8[sh].data(), aligned_policy{}) };
}

template <simd_type V, class Int>
raze_nodiscard raze_always_inline V slide_right_fallback_(const V& x, Int sh) noexcept {
    alignas(sizeof(V)) typename V::value_type arr[V::size() * 2];

    vx::store[vx::aligned](arr, V::zero());
    vx::store[vx::aligned](arr + V::size(), x);

    return vx::load<V>[vx::aligned](arr + (V::size() - sh));
}

template <intrin_type V, class Pattern>
raze_always_inline V slide_right_native_(V x, Pattern p) noexcept {
    constexpr auto isa = abi_t<pattern_vector_t<Pattern>>::isa;
    using Value = typename pattern_vector_t<Pattern>::value_type;

    constexpr auto shift = get_slide_right_shift(p) == shuffle_zero ? p.size() : get_slide_right_shift(p);
    constexpr auto shift_bytes = shift * sizeof(Value);
    constexpr auto size = p.size();

    if constexpr (shift == 0)
        return x;

    if constexpr (sizeof(V) == 16) return as<V>(_mm_slli_si128(as<__m128i>(x), shift_bytes));
    else if constexpr (sizeof(V) == 32 && has_avx2<isa>) {
        if constexpr (has_avx512vl<isa> && (shift_bytes % 4) == 0) {
            return as<V>(_mm256_alignr_epi32(as<__m256i>(x), _mm256_setzero_si256(), (8 - (shift_bytes >> 2)) & 7));
        }
        else {
            auto low_part = _mm256_setzero_si256();
            auto high_part = _mm256_setzero_si256();

            if constexpr (shift_bytes >= sizeof(V)) return zero_<isa, V>();
            else if constexpr (shift_bytes == 0) return x;
            else if constexpr (shift_bytes < 16) {
                low_part = as<__m256i>(x);
                high_part = _mm256_inserti128_si256(high_part, as<__m128i>(x), 1);
            }
            else if constexpr (shift_bytes < 32) low_part = _mm256_inserti128_si256(low_part, as<__m128i>(x), 1);
            else return zero_<isa, V>();

            if constexpr ((shift_bytes % 16) == 0) return as<V>(low_part);
            return as<V>(_mm256_alignr_epi8(low_part, high_part, 16 - (shift_bytes & 0xF)));
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (shift_bytes >= sizeof(V)) return zero_<isa, V>();

        auto low_part = _mm512_setzero_si512();
        auto high_part = _mm512_setzero_si512();

        if constexpr (shift_bytes == 0) return x;
        else if constexpr (shift_bytes < 16) {
            low_part = as<__m512i>(x);
            high_part = _mm512_maskz_shuffle_i64x2(0xFC, as<__m512i>(x), as<__m512i>(x), 0x90);
        }
        else if constexpr (shift_bytes < 32) {
            low_part = _mm512_maskz_shuffle_i64x2(0xFC, as<__m512i>(x), as<__m512i>(x), 0x90);
            high_part = _mm512_maskz_shuffle_i64x2(0xF0, as<__m512i>(x), as<__m512i>(x), 0x40);
        }
        else if constexpr (shift_bytes < 48) {
            low_part = _mm512_maskz_shuffle_i64x2(0xF0, as<__m512i>(x), as<__m512i>(x), 0x40);
            high_part = _mm512_maskz_shuffle_i64x2(0xC0, as<__m512i>(x), as<__m512i>(x), 0x00);
        }
        else if constexpr (shift_bytes < 64) {
            low_part = _mm512_maskz_shuffle_i64x2(0xC0, as<__m512i>(x), as<__m512i>(x), 0x00);
        }
        else return zero_<isa, V>();

        if constexpr ((shift_bytes % 4) == 0) return as<V>(_mm512_alignr_epi32(
            as<__m512i>(x), _mm512_setzero_si512(), (16 - (shift_bytes >> 2)) & 15));

        if constexpr (has_avx512bw<isa>) return as<V>(_mm512_alignr_epi8(low_part, high_part, 16 - (shift_bytes & 0xF)));
        else {
            const auto low = _mm256_alignr_epi8(as<__m256i>(low_part), as<__m256i>(high_part), 16 - (shift_bytes & 0xF));
            const auto high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(as<__m512i>(low_part), 1),
                _mm512_extracti64x4_epi64(as<__m512i>(high_part), 1), 16 - (shift_bytes & 0xF));

            return as<V>(_mm512_inserti64x4(as<__m512i>(low), high, 1));
        }
    }
    else {
        alignas(sizeof(V)) Value arr[p.size() * 2];

        store_(arr, zero_<isa, V>(), aligned_policy{});
        store_(arr + p.size(), x, aligned_policy{});

        return load_<isa, V>(arr + (p.size() - shift), aligned_policy{});
    }
}

template <class Pattern>
raze_always_inline pattern_vector_t<Pattern> slide_right_(const pattern_vector_t<Pattern>& x, Pattern p) noexcept {
    using V = pattern_vector_t<Pattern>;

    if constexpr (get_slide_right_shift(p) == 0) return x;
    else if constexpr (get_slide_right_shift(p) >= p.size()) return V::zero();
    else if constexpr (native<V>) {
        auto r = x;

        auto& storage = r.template __get<0>();
        storage = slide_right_native_(ustorage(x.template __get<0>()), p);

        return r;
    }
    else return slide_right_fallback_(x, get_slide_right_shift(p) == shuffle_zero ? p.size() : get_slide_right_shift(p));
}

template <simd_type V>
raze_always_inline V slide_right_(const V& x, i32 sh) noexcept {
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

        using Intrin = decltype(ustorage(x.template __get<0>()));
        using RetSlide = decltype(make_pshufb_slide_right_idx_<isa, Value>(Intrin{}, sh));
        using IdxType = typename RetSlide::index_type;

        using Ret = decltype(generic_shuffle_native_<isa, IdxType>(Intrin{}, std::declval<RetSlide>().data()));

        if constexpr (is_fallback<Ret>) return slide_right_fallback_(x, sh);
        else {
            auto r = x;
            auto& storage = r.template __get<0>();

            const auto slide_indices = make_pshufb_slide_right_idx_<isa, Value>(ustorage(storage), sh);
            using IndexType = typename decltype(slide_indices)::index_type;

            storage = generic_shuffle_native_<isa, IndexType>(ustorage(storage), slide_indices.data());

            return r;
        }
    }
    else return slide_right_fallback_(x, sh);
}

__RAZE_VX_NAMESPACE_END
