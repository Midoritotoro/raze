#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Storage.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Fms.h>
#endif // defined(raze_processor_x86)


__RAZE_MATH_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_fms: raze::options::strict_elementwise_callable<_Configurable_fms, _Options_> {
    template <class _A_, class _B_, class _C_>
        requires (vx::simd_type<std::remove_cvref_t<_A_>> ||
            vx::simd_type<std::remove_cvref_t<_B_>> ||
            vx::simd_type<std::remove_cvref_t<_C_>>)
    raze_nodiscard raze_always_inline std::conditional_t<vx::simd_type<std::remove_cvref_t<_A_>>, std::remove_cvref_t<_A_>,
        std::conditional_t<vx::simd_type<std::remove_cvref_t<_B_>>, std::remove_cvref_t<_B_>,
        std::remove_cvref_t<_C_>>> operator()(_A_&& __x, _B_&& __y, _C_&& __z) const noexcept {
        using _Simd_ = std::conditional_t<vx::simd_type<std::remove_cvref_t<_A_>>, std::remove_cvref_t<_A_>,
            std::conditional_t<vx::simd_type<std::remove_cvref_t<_B_>>, std::remove_cvref_t<_B_>,
            std::remove_cvref_t<_C_>>>;

        return raze::options::__dispatch_call(*this,
            _Simd_(std::forward<_A_>(__x)),
            _Simd_(std::forward<_B_>(__y)),
            _Simd_(std::forward<_C_>(__z)));
    }

    template <vx::arithmetic_type _Type_>
    raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __x, const _Type_& __y, const _Type_& __z) const noexcept {
        return raze::options::__dispatch_call(*this, __x, __y, __z);
    }

    template <vx::arithmetic_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, const _Type_& __y, const _Type_& __z) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                return vx::_Fms<arch::ISA::SSE2, _Type_>()(__x, __y, __z, __mask, __condition.alternative());
            else
                return vx::_Fms<arch::ISA::SSE2, _Type_>()(__x, __y, __z, __mask);
        }
        else return vx::_Fms<arch::ISA::SSE2, _Type_>()(__x, __y, __z);
    }

    template <vx::simd_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, const _Type_& __y, const _Type_& __z) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        _Type_ __result = __x;

        auto __chunk_op = [&] <class _Chunk, class ... _Args_> (_Chunk& __chunk, _Args_&& ... __args) raze_always_inline_lambda {
            __chunk = vx::_Fms<_Abi_::isa, _Value_>()(vx::__storage_unwrap(__chunk), vx::__storage_unwrap<_Args_>(__args)...);
        };

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                __result.__for_each_chunk(__chunk_op, __y.__storage().storage(), __z.__storage().storage(), __mask.__storage().storage(),
                     __condition.alternative().__storage().storage());
            else
                __result.__for_each_chunk(__chunk_op, __y.__storage().storage(), __z.__storage().storage(), __mask.__storage().storage());
        }
        else {
            __result.__for_each_chunk(__chunk_op, __y.__storage().storage(), __z.__storage().storage());
        }

        return __result;
    }

    using callable_tag_type = _Configurable_fms;
};

constexpr inline auto __fms = raze::options::functor<_Configurable_fms>;

__RAZE_MATH_NAMESPACE_END

