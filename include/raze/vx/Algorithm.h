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
#include <src/raze/vx/hw/Shape.h>

__RAZE_VX_NAMESPACE_BEGIN

/* abs, fma, fms, fnma, fnms, sin, cos defined in raze/math/Math.h */

/**
 * @class counter
 * @brief Accumulates set-bit counts across `simd_mask<V>` instances.
 *
 * Folds a sequence of masks into a running total reported by `result()`.
 *
 * ## Overflow
 *
 * `count()` performs no overflow check. The maximum number of
 * `count()` calls a single instance absorbs without wrap is
 * `portion_size()`. Beyond that bound, lane accumulation wraps in
 * two's-complement arithmetic. To process more than `portion_size()`
 * masks, construct a fresh `counter<V>` per portion and accumulate
 * `result()` into a wider scalar.
 *
 * ## State
 *
 * No reset operation. `result()` does not modify state.
 *
 * ## Template Parameters
 *
 * @tparam V  Vector type providing `mask_type` and `value_type`.
 *            Constraint: `trivially_chunk_swappable`.
 *
 * ## Type aliases
 *
 * - `value_type` — `V::value_type`.
 * - `mask_type`  — `V::mask_type`.
 * - `index_type` — `IntegerForSizeof<value_type>::Signed`.
*/
template <trivially_chunk_swappable V>
class counter {
    using storage_type = std::conditional_t<(has_avx512f<abi_t<V>::isa> && sizeof(value_type) >= 4)
        || (has_avx512bw<abi_t<V>::isa>), sizetype, simd<index_type, abi_t<V>>>;

    storage_type _storage;
public:
    using mask_type = typename V::mask_type;
    using value_type = typename V::value_type;
    using index_type = typename IntegerForSizeof<value_type>::Signed;
    
    counter() noexcept {
        if constexpr (simd_type<storage_type>) _storage = zero<storage_type>();
        else _storage = 0;
    }

    raze_always_inline static constexpr auto portion_size() noexcept {
        return math::max_limit<index_type>();
    }

    raze_always_inline void count(const mask_type& mask) noexcept {
        if constexpr (std::integral<storage_type>) _storage += count_set(mask);
        else _storage.__for_each_chunk([&] <class Chunk, class Chunk2> (Chunk& chunk, const Chunk2& ch2) raze_always_inline_lambda {
            chunk = sub_<abi_t<V>::isa, index_type>(ustorage(chunk), ustorage(ch2));
        }, mask.__storage().storage());
    }

    raze_always_inline auto result() const noexcept {
        if constexpr (std::integral<storage_type>) return _storage;
        else return horizontal_sum(_storage);
    }
};

__RAZE_VX_NAMESPACE_END
