#pragma once 

#include <raze/options/Options.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    template <auto, class> class    _OpFunctor_,
    class ...                       _SupportedOptions_>
struct _Configurable_unary_operation_base {
    template <class _Options_>
    struct __impl: public raze::options::strict_elementwise_callable<__impl, _Options_, _SupportedOptions_...> {
        template <simd_type _Simd_>
        static constexpr auto __op = _OpFunctor_<_Simd_::__isa, typename _Simd_::value_type>{};

        template <simd_type _Simd_>
        using op_return_type = decltype(__op<_Simd_>(std::declval<typename _Simd_::vector_type>()));

        template <simd_type _Simd_>
        using return_type = std::conditional_t<std::is_arithmetic_v<op_return_type<_Simd_>>, op_return_type<_Simd_>, _Simd_>;
        
        using mask_type = raze::options::fetch_t<raze::options::condition_key, _Options_>;

        template <simd_type _Simd_>
        raze_nodiscard raze_always_inline return_type<_Simd_> operator()(const _Simd_& __x) const noexcept {
            return raze::options::__dispatch_call(*this, __x);
        }

        template <simd_type _Simd_>
        static raze_always_inline return_type<_Simd_> deferred_call(
            auto            __options,
            const _Simd_&   __x) noexcept
        {
            if constexpr (options::concepts::same_as<mask_type, options::unknown_key>) {
                return __op<_Simd_>(__data(__x));
            }
            else {
                auto __condition = __options[raze::options::condition_key];
                const auto __mask = __condition.mask(raze::options::as<typename mask_type::condition_type>{});

                if constexpr (mask_type::has_alternative)
                    return __op<_Simd_>(__data(__x), __data(__mask), __data(__condition.alternative()));
                else
                    return __op<_Simd_>(__data(__x), __data(__mask));
            }
        }

        using callable_tag_type = __impl;
    };
};


template <
    template <auto, class> class    _OpFunctor_,
    class                           _Options_,
    class ...                       _SupportedOptions_>
using __configurable_unary_operation_type = typename _Configurable_unary_operation_base<_OpFunctor_, _SupportedOptions_...>::template __impl<_Options_>;

__RAZE_VX_NAMESPACE_END
