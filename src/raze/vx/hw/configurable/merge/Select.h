#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/merge/Select.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_select_t: options::conditional_callable<configurable_select_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline V operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<raze::options::condition_key, _Options_>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        V r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = select_<Abi::isa, Value>()(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (!std::same_as<Mask, options::unknown_key> && !std::same_as<Mask, options::ignore_none_>) {
            auto condition = opts[options::condition_key];
            const auto mask = condition.mask();

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, condition.alternative().__storage().storage(), mask.__storage().storage());
            else
                r.__for_each_chunk(chunk_op, mask.__storage().storage());
        }
        
        return r;
    }
};

constexpr inline auto select = raze::options::functor<configurable_select_t>;

__RAZE_VX_NAMESPACE_END
