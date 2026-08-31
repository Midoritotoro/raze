#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_none_of_t: options::conditional_callable<configurable_none_of_t, Options> {
    template <simd_mask_type V>
    raze_nodiscard raze_always_inline bool operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    raze_nodiscard raze_always_inline auto operator()(algorithm::tail_mask_type auto const& x) const noexcept {
        return (*this)(x());
    }

    template <simd_mask_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        auto chunk_op = [&] <class Chunk, class ... Args> (const Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            return none_of_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            static_assert(!Mask::has_alternative, "Not supported. ");

            auto condition = opts[options::condition_key];
            return x.__for_each_chunk_all_of(chunk_op, condition.mask().__storage().storage());
        }
        else {
            return x.__for_each_chunk_all_of(chunk_op);
        }
    }
};

constexpr inline auto none_of = options::functor<configurable_none_of_t>;

__RAZE_VX_NAMESPACE_END
