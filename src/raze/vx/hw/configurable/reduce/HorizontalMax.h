#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/reduce/HorizontalMax.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_hmax_t: options::conditional_callable<configurable_hmax_t, Options> {
    template <simd_type V>
    raze_nodiscard raze_always_inline typename V::value_type operator()(const V& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        Value r = 0;
        
        auto chunk_op = [&] <class ... Args> (Args&& ... args) raze_always_inline_lambda {
            r = std::max(horizontal_max_<Abi::isa, Value>(ustorage<Args>(args)...), r);
        };

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative) {
                auto alternative = condition.alternative();
                r = condition.mask()[std::integral_constant<sizetype, 0>{}] ? x[std::integral_constant<sizetype, 0>{}] : alternative[std::integral_constant<sizetype, 0>{}];
                x.__for_each_chunk(chunk_op, condition.mask().__storage().storage(), alternative.__storage().storage());
            }
            else {
                r = condition.mask()[std::integral_constant<sizetype, 0>{}] ? x[std::integral_constant<sizetype, 0>{}] : 0;
                x.__for_each_chunk(chunk_op, condition.mask().__storage().storage());
            }
        }
        else {
            r = x[std::integral_constant<sizetype, 0>{}];
            x.__for_each_chunk(chunk_op);
        }

        return r;
    }
};

constexpr inline auto horizontal_max = options::functor<configurable_hmax_t>;
constexpr inline auto hmax = horizontal_max;

__RAZE_VX_NAMESPACE_END
