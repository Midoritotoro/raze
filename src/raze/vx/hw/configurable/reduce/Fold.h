#pragma once 

#include <raze/options/Options.h>
#include <src/raze/traits/FunctionPass.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/vx/hw/configurable/arithmetic/Add.h>
#include <src/raze/vx/hw/configurable/reduce/HorizontalSum.h>
#include <src/raze/vx/hw/configurable/shuffle/SwapAdjacent.h>
#include <src/raze/vx/hw/configurable/merge/Select.h>
#include <src/raze/traits/FunctionPass.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/reduce/Fold.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

struct broadcast_mode {};
constexpr inline auto broadcast = raze::options::flag(broadcast_mode{});
struct broadcast_option : raze::options::exact_option<broadcast> {};

template <class Options>
struct configurable_fold_t : options::conditional_callable<configurable_fold_t, Options, broadcast_option> {
    template <simd_type V, class F>
    raze_nodiscard raze_always_inline V operator()(const V& x, F f) const noexcept
        requires(Options::contains(broadcast))
    {
        return options::dispatch_call(*this, x, traits::fwd_fn(f));
    }

    template <simd_type V, class F>
    raze_nodiscard raze_always_inline typename V::value_type operator()(const V& x, F f) const noexcept
        requires(!Options::contains(broadcast))
    {
        return options::dispatch_call(*this, x, traits::fwd_fn(f));
    }

    template <simd_type V, class F>
    static raze_always_inline auto deferred_call(auto opts, const V& x, F f) noexcept
        requires(std::is_same_v<traits::function_unwrapped<std::remove_cvref_t<F>>, std::remove_cvref_t<decltype(add)>>)
    {
        return hsum[opts](x);
    }

    template <simd_type V, class F>
    static raze_always_inline auto deferred_call(auto opts, const V& x, F f) noexcept
        requires(!std::is_same_v<traits::function_unwrapped<std::remove_cvref_t<F>>, std::remove_cvref_t<decltype(add)>>)
    {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        if constexpr (V::size() == 1) {
            if constexpr (Options::contains(broadcast)) return select[opts](x);
            else return Value(select[opts](x)[std::integral_constant<sizetype, 0>{}]);
        }
        else {
            constexpr auto depth = std::bit_width(sizetype(V::size())) - 1;

            const auto r = [&] <sizetype ... Indices> (std::integer_sequence<sizetype, Indices...>) raze_always_inline_lambda {
                x = select[opts](x);
                ((x = f(x, swap_adjacent_(x, std::integral_constant<sizetype, 1ull << Indices>{}))), ...);
                return x;
            } (std::make_integer_sequence<sizetype, depth>{});

            if constexpr (Options::contains(broadcast)) return r;
            else return r[std::integral_constant<sizetype, 0>{}];
        }
    }
};

constexpr inline auto fold = options::functor<configurable_fold_t>;

__RAZE_VX_NAMESPACE_END
