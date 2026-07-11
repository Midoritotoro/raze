#pragma once 

#include <raze/options/Options.h>
#include <src/raze/type_traits/FunctionPass.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/vx/hw/configurable/arithmetic/Add.h>
#include <src/raze/vx/hw/configurable/reduce/HorizontalSum.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/reduce/Fold.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_fold : raze::options::strict_elementwise_callable<_Configurable_fold, _Options_> {
    template <simd_type _Type_, class _Callable_>
    raze_nodiscard raze_always_inline typename _Type_::value_type operator()(const _Type_& __x, _Callable_ __callable) const noexcept {
        return raze::options::__dispatch_call(*this, __x, type_traits::__pass_function(__callable));
    }

    template <simd_type _Type_, class _Callable_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, _Callable_ __callable) noexcept {
        /*if constexpr (std::is_same_v<algorithm::__function_unwrapped<_Callable_>, std::remove_cvref_t<decltype(__add)>>) {
            return __hsum[__options](__x);
        }
        else*/ {
            using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
            using _Value_ = typename _Type_::value_type;
            using _Abi_ = typename _Type_::abi_type;

            if constexpr (_Type_::size() == 1) {
                if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
                    auto __condition = __options[raze::options::condition_key];
                    const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

                    if constexpr (_Mask_::has_alternative)
                        return __mask[std::integral_constant<sizetype, 0>{}] ? __x[std::integral_constant<sizetype, 0>{}] : __condition.alternative()[std::integral_constant<sizetype, 0>{}];
                    else
                        return __mask[std::integral_constant<sizetype, 0>{}] ? __x[std::integral_constant<sizetype, 0>{}] : 0;
                }
                else {
                    return __x[std::integral_constant<sizetype, 0>{}];
                }
            }
            else {
                _Value_ __result;

                auto __wrapped_callable = [&] <class ... _Args_> (_Args_&& ... __args) raze_always_inline_lambda {
                    return __callable(options::as<_Value_>{}, std::integral_constant<arch::ISA, _Abi_::isa>{}, std::forward<_Args_>(__args)...);
                };

                auto __chunk_op = [&] <class ... _Args_> (_Args_&& ... __args) raze_always_inline_lambda {
                    __result = __callable(_Fold<_Abi_::isa, _Value_>()(__storage_unwrap<_Args_>(__args)...), __result);
                };

                if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
                    auto __condition = __options[raze::options::condition_key];
                    const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

                    if constexpr (_Mask_::has_alternative) {
                        __result = __mask[std::integral_constant<sizetype, 0>{}] ? __x[std::integral_constant<sizetype, 0>{}] : __condition.alternative()[std::integral_constant<sizetype, 0>{}];
                        __x.__for_each_chunk(__chunk_op, __wrapped_callable, __mask.__storage().storage(), __condition.alternative().__storage().storage());
                    }
                    else {
                        __result = __mask[std::integral_constant<sizetype, 0>{}] ? __x[std::integral_constant<sizetype, 0>{}] : 0;
                        __x.__for_each_chunk(__chunk_op, __wrapped_callable, __mask.__storage().storage());
                    }
                }
                else {
                    __result = __x[std::integral_constant<sizetype, 0>{}];
                    __x.__for_each_chunk(__chunk_op, __wrapped_callable);
                }

                return __result;
            }
        }
    }

    using callable_tag_type = _Configurable_fold;
};

__RAZE_VX_NAMESPACE_END
