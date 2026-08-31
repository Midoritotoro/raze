#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/mask/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/AnyOf.h>
#  include <src/raze/vx/hw/x86/mask/operations/ClearLast.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_clear_last_t : options::conditional_callable<configurable_clear_last_t, Options, not_null_option> {
    template <simd_mask_type M>
    raze_nodiscard raze_always_inline M operator()(const M& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    raze_nodiscard raze_always_inline auto operator()(algorithm::tail_mask_type auto const& x) const noexcept {
        return (*this)(x());
    }

    template <simd_mask_type M>
    static raze_always_inline auto deferred_call(auto opts, const M& x) noexcept {
        using Value = typename M::value_type;
        using Abi = typename M::abi_type;

        constexpr auto is_unsafe = Options::contains(not_null) && M::__chunks_count() == 1;

        M r = x;
        r.__for_each_chunk_any_of_reverse([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
            if constexpr (is_unsafe) {
                chunk = clear_last_<Abi::isa, Value, Chunk::size, is_unsafe>(ustorage(chunk));
                return true;
            }
            else {
                if (any_of_<Abi::isa, Value>(ustorage(chunk))) {
                    chunk = clear_last_<Abi::isa, Value, Chunk::size, is_unsafe>(ustorage(chunk));
                    return true;
                }
                else return false;
            }
        });

        return r;
    }
};

constexpr inline auto clear_last = options::functor<configurable_clear_last_t>;

__RAZE_VX_NAMESPACE_END
