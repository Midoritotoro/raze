#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/mask/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/FindNextSet.h>
#  include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_find_next_set_t : raze::options::conditional_callable<configurable_find_next_set_t, Options, not_null_option> {
    template <simd_mask_type M>
    raze_nodiscard raze_always_inline i32 operator()(const M& x, i32 from) const noexcept {
        return options::dispatch_call(*this, x, from);
    }

    raze_nodiscard raze_always_inline auto operator()(algorithm::tail_mask_type auto const& x, i32 from) const noexcept {
        return (*this)(x(), from);
    }

    template <simd_mask_type M>
    static raze_always_inline auto deferred_call(auto opts, const M& x, i32 from) noexcept {
        using Value = typename M::value_type;
        using Abi = typename M::abi_type;

        i32 index = 0;
        constexpr auto is_unsafe = Options::contains(not_null) && M::__chunks_count() == 1;

        i32 shift = from < 0 ? 0u : u32(from + 1);

        auto chunk_op = [&] <class Chunk> (const Chunk& chunk) raze_always_inline_lambda {
            if (shift >= i32(Chunk::size)) {
                index += Chunk::size;
                shift -= Chunk::size;
                return true;
            }

            auto r = find_next_set_<Abi::isa, Chunk::size, Value, is_unsafe>(ustorage(chunk), shift);
            index += r;

            if (r != Chunk::size) return false;

            shift = 0;
            return true;
        };

        x.__for_each_chunk_all_of(chunk_op);
        return index;
    }
};

/**
 * @ingroup raze_vx_mask_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Finds the index of the first set lane strictly after a given position.
 *
 * Scans the input mask `x` starting from `from + 1` upwards and returns the index of the first lane
 * that is `true`. If `from < 0`, the search starts from lane `0`. If no `true` lane is found,
 * returns `M::size()`. Also accepts `algorithm::tail_mask_type` by delegating to its underlying mask.
 *
 * ### Declarations
 * ```cpp
 * template <simd_mask_type M>
 * i32 operator()(const M& x, i32 from) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `M` — The source SIMD mask type. Defines the lane count, element type, and ABI.
 *
 * ### Parameters
 * - `x` — The input mask to inspect.
 * - `from` — The position *after* which to start the search.
 *
 * ### Options
 * - `raze::vx::not_null` — Assumes there is a set lane after `from`.
 *   This skips the internal checks and uses a faster, unsafe implementation.
 *   If no such lane exists, behavior is undefined.
 *
 * ### Mask semantics
 * **Not supported.** 
 *
 * ### Return value
 * The 0-based index of the first `true` lane after `from`, or `M::size()` if no such lane exists.
*/
constexpr inline auto find_next_set = options::functor<configurable_find_next_set_t>;

__RAZE_VX_NAMESPACE_END
