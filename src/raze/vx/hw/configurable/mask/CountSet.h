#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/CountSet.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_count_set_t: options::conditional_callable<configurable_count_set_t, Options> {
    template <simd_mask_type M>
    raze_nodiscard raze_always_inline i32 operator()(const M& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    raze_nodiscard raze_always_inline auto operator()(algorithm::tail_mask_type auto const& x) const noexcept {
        return (*this)(x());
    }

    template <simd_mask_type M>
    static raze_always_inline auto deferred_call(auto opts, const M& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename M::value_type;
        using Abi = typename M::abi_type;

        auto count = 0;

        auto chunk_op = [&] <class Chunk, class ... Args> (const Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            count += count_set_<Abi::isa, Chunk::size, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            static_assert(!Mask::has_alternative, "Not supported. ");
            auto condition = opts[options::condition_key];
            x.__for_each_chunk(chunk_op, condition.mask().__storage().storage());
        }
        else {
            x.__for_each_chunk(chunk_op);
        }

        return count;
    }
};

constexpr inline auto count_set = options::functor<configurable_count_set_t>;

__RAZE_VX_NAMESPACE_END
