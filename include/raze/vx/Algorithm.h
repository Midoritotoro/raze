#pragma once 

#include <src/raze/vx/hw/Reduce.h>
#include <src/raze/vx/hw/Compact.h>
#include <src/raze/vx/hw/Compare.h>
#include <src/raze/vx/hw/Construct.h>
#include <src/raze/vx/hw/Shuffle.h>
#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/vx/hw/Bitwise.h>
#include <src/raze/vx/hw/Mask.h>
#include <src/raze/vx/hw/Merge.h>
#include <src/raze/vx/hw/Memory.h>
#include <src/raze/vx/AsTernaryMask.h>
#include <src/raze/vx/hw/Shape.h>

__RAZE_VX_NAMESPACE_BEGIN

/* abs, fma, fms, fnma, fnms, sin, cos defined in raze/math/Math.h */

template <ternary_mask_expression_type Expression>
raze_always_inline constexpr auto as_ternary_mask() noexcept {
	return as_ternary_mask_impl<Expression>();
}

template <trivially_chunk_swappable V>
struct counter {
    using mask_type = typename V::mask_type;
    using value_type = typename V::value_type;
    using index_type = typename IntegerForSizeof<value_type>::Signed;
    using storage_type = std::conditional_t<(has_avx512f<abi_t<V>::isa> && sizeof(value_type) >= 4)
        || (has_avx512bw<abi_t<V>::isa>), sizetype, simd<index_type, abi_t<V>>>;

    counter() noexcept {
        if constexpr (simd_type<storage_type>) _storage = storage_type::zero();
        else _storage = 0;
    }

    raze_always_inline static constexpr auto portion_size() noexcept {
        return math::max_limit<index_type>();
    }

    raze_always_inline void count(const mask_type& mask) noexcept {
        if constexpr (std::integral<storage_type>) _storage += count_set(mask);
        else _storage.__for_each_chunk([&] <class _Chunk_, class _Chunk2_> (_Chunk_ & chunk, const _Chunk2_ & ch2) raze_always_inline_lambda {
            chunk = _Sub<abi_t<V>::isa, index_type>()(ustorage(chunk), ustorage(ch2));
        }, mask.__storage().storage());
    }

    raze_always_inline auto result() const noexcept {
        if constexpr (std::integral<storage_type>) return _storage;
        else return horizontal_sum(_storage);
    }

    storage_type _storage;
};

__RAZE_VX_NAMESPACE_END
