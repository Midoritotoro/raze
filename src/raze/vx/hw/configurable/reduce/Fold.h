#pragma once 

#include <raze/options/Options.h>
#include <src/raze/type_traits/FunctionPass.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/vx/hw/configurable/arithmetic/Add.h>
#include <src/raze/vx/hw/configurable/reduce/HorizontalSum.h>
#include <src/raze/vx/hw/configurable/shuffle/SwapAdjacent.h>
#include <src/raze/vx/hw/configurable/merge/Select.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/reduce/Fold.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

struct broadcast_mode {};
constexpr inline auto broadcast = raze::options::flag(broadcast_mode{});
struct broadcast_option : raze::options::exact_option<broadcast> {};

template <class _Options_>
struct _Configurable_fold : raze::options::strict_elementwise_callable<_Configurable_fold, _Options_, broadcast_option> {
    template <simd_type _Type_, class _Callable_>
    raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __x, _Callable_ __callable) const noexcept
        requires(_Options_::contains(broadcast))
    {
        return raze::options::__dispatch_call(*this, __x, type_traits::__pass_function(__callable));
    }

    template <simd_type _Type_, class _Callable_>
    raze_nodiscard raze_always_inline typename _Type_::value_type operator()(const _Type_& __x, _Callable_ __callable) const noexcept
        requires(!_Options_::contains(broadcast))
    {
        return raze::options::__dispatch_call(*this, __x, type_traits::__pass_function(__callable));
    }

    template <simd_type _Type_, class _Callable_>
    static raze_always_inline auto deferred_call(auto __options, _Type_ __x, _Callable_ __callable) noexcept {
            using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
            using _Value_ = typename _Type_::value_type;
            using _Abi_ = typename _Type_::abi_type;

            if constexpr (_Type_::size() == 1) {
                if constexpr (_Options_::contains(broadcast)) return __select[__options](__x);
                else return __select[__options](__x)[std::integral_constant<sizetype, 0>{}];
            }
            else {
                constexpr auto __depth = std::bit_width(sizetype(_Type_::size())) - 1;

                const auto __r = [&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda {
                    __x = __select[__options](__x);
                    ((__x = __callable(__x, __swap_adjacent(__x, std::integral_constant<sizetype, 1ull << _Indices_>{}))), ...);
                    return __x;
                } (std::make_integer_sequence<sizetype, __depth>{});

                if constexpr (_Options_::contains(broadcast)) return __r;
                else return __r[std::integral_constant<sizetype, 0>{}];
            }
    }

    using callable_tag_type = _Configurable_fold;
};

__RAZE_VX_NAMESPACE_END
