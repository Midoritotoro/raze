#pragma once 

#include <raze/options/Options.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    template <auto, auto, class> class  _OpFunctor_,
    class ...                           _SupportedOptions_>
struct _Configurable_binary_operation_base {
    template <class _Options_>
    struct __impl : public raze::options::strict_elementwise_callable<__impl, _Options_, _SupportedOptions_...> {
        template <simd_type _Simd_>
        static constexpr auto __op = _OpFunctor_<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>{};

        template <simd_type _Simd_>
        raze_nodiscard raze_always_inline _Simd_ operator()(
            const _Simd_& __x, 
            const _Simd_& __y) const noexcept 
        {
            return raze::options::__dispatch_call(*this, __x, __y);
        }

        template <simd_type _Simd_>
        raze_nodiscard raze_always_inline _Simd_ operator()(
            const _Simd_&               __x,
            typename _Simd_::value_type __y) const noexcept
        {
            return raze::options::__dispatch_call(*this, __x, __y);
        }
        
        template <simd_type _Simd_>
        raze_nodiscard raze_always_inline _Simd_ operator()(
            typename _Simd_::value_type __x,
            const _Simd_&               __y) const noexcept
        {
            return raze::options::__dispatch_call(*this, _Simd_(__x), __y);
        }

        template <simd_type _Simd_>
        static raze_always_inline auto deferred_call(
            auto            __options,
            const _Simd_&   __x,
            const _Simd_&   __y) noexcept 
        {
            using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;

            if constexpr (options::concepts::same_as<_Mask_, options::unknown_key>) {
                return __op<_Simd_>(__data(__x), __data(__y));
            }
            else {
                auto __condition = __options[raze::options::condition_key];
                const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

                if constexpr (_Mask_::has_alternative)
                    return __op<_Simd_>(__data(__x), __data(__y), __data(__mask), __data(__condition.alternative()));
                else
                    return __op<_Simd_>(__data(__x), __data(__y), __data(__mask));
            }
        }

        template <simd_type _Simd_>
        static raze_always_inline auto deferred_call(
            auto                        __options,
            const _Simd_&               __x,
            typename _Simd_::value_type __y) noexcept
        {
            using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;

            auto __rhs_data = [&] {
                if constexpr (requires { __op<_Simd_>(__data(__x), __y); })
                    return __y;
                else 
                    return __data(_Simd_(__y));
            }();

            if constexpr (options::concepts::same_as<_Mask_, options::unknown_key>) {
                return __op<_Simd_>(__data(__x), __rhs_data);
            }
            else {
                auto __condition = __options[raze::options::condition_key];
                const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

                if constexpr (_Mask_::has_alternative)
                    return __op<_Simd_>(__data(__x), __rhs_data, __data(__mask), __data(__condition.alternative()));
                else
                    return __op<_Simd_>(__data(__x), __rhs_data, __data(__mask));
            }
        }

        using callable_tag_type = __impl;
    };
};


template <
    template <auto, auto, class> class  _OpFunctor_,
    class                               _Options_,
    class ...                           _SupportedOptions_>
using __configurable_binary_operation_type = typename _Configurable_binary_operation_base<_OpFunctor_, _SupportedOptions_...>::template __impl<_Options_>;

__RAZE_VX_NAMESPACE_END
