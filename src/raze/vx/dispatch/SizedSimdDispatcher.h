#pragma once 

#include <raze/vx/Simd.h>
#include <raze/options/Options.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Fn_, class _Tuple_, size_t... _I_>
raze_always_inline constexpr decltype(auto) my_apply(_Fn_&& __fn, _Tuple_&& __t, std::index_sequence<_I_...>) noexcept {
    return std::forward<_Fn_>(__fn)(std::get<_I_>(std::forward<_Tuple_>(__t))...);
}

template <class _Callable_, class _Tuple_>
raze_always_inline constexpr decltype(auto) my_apply(_Callable_&& __obj, _Tuple_&& __tup) noexcept {
    return vx::my_apply(std::forward<_Callable_>(__obj), std::forward<_Tuple_>(__tup),
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<_Tuple_>>>{});
}

template <class _Fn_, class _Tuple_>
raze_always_inline auto unpack(_Fn_&& __fn, _Tuple_&& __tuple) noexcept {
    return my_apply(std::forward<_Fn_>(__fn), std::forward<_Tuple_>(__tuple));
}

template <class _Fn_, class _Tuple_, class... _Suffix_>
raze_always_inline auto unpack_suffix(_Fn_&& __fn, _Tuple_&& __tuple, _Suffix_&&... __suffix) noexcept {
    return my_apply([&] <class... _E_> (_E_&&... __elements) raze_always_inline_lambda {
        return std::forward<_Fn_>(__fn)(std::forward<_E_>(__elements)..., std::forward<_Suffix_>(__suffix)...);
    }, std::forward<_Tuple_>(__tuple));
}

template <template <class> class _Function_, class _Type_, class _Return_>
struct _Configurable_sized_isa_dispatcher {
    template <class _Options_>
    struct __impl : raze::options::strict_elementwise_callable<__impl, _Options_>{
        template <class _Size_, class ... _Args_>
        raze_always_inline _Return_ operator()(_Size_ __size, _Args_&& ... __args) const noexcept
        {
            return raze::options::__dispatch_call(*this, __size, std::forward<_Args_>(__args)...);
        }

        template <class _Size_, class ... _Args_>
        static raze_always_inline _Return_ deferred_call(auto __options,
            _Size_ __size, _Args_&& ... __args) noexcept
        {
           if (const auto __aligned_size = __size & ~0x3F; __aligned_size != 0) {
                if constexpr (sizeof(_Type_) >= 4) {
                    if (arch::ProcessorFeatures::AVX512F()) {
                        using _Simd_ = simd<_Type_, runtime_abi<arch::ISA::AVX512F, 64 / sizeof(_Type_)>>;
                        return _Function_<_Simd_>()(std::forward<_Args_>(__args)..., __aligned_size, __size - __aligned_size);
                    }
                }
                else {
                    if (arch::ProcessorFeatures::AVX512BW()) {
                        using _Simd_ = simd<_Type_, runtime_abi<arch::ISA::AVX512BW, 64 / sizeof(_Type_)>>;
                        return _Function_<_Simd_>()(std::forward<_Args_>(__args)..., __aligned_size, __size - __aligned_size);
                    }
                }
            }

            if (const auto __aligned_size = __size & ~0x1F; __aligned_size != 0 && arch::ProcessorFeatures::AVX2()) {
                using _Simd_ = simd<_Type_, runtime_abi<arch::ISA::AVX2, 32 / sizeof(_Type_)>>;
                return _Function_<_Simd_>()(std::forward<_Args_>(__args)..., __aligned_size, __size - __aligned_size);
            }

            else if (const auto __aligned_size = __size & ~0xF; __aligned_size != 0 && arch::ProcessorFeatures::SSE42())
                return _Function_<simd<_Type_, runtime_abi<arch::ISA::SSE42, 16 / sizeof(_Type_)>>>()(
                    std::forward<_Args_>(__args)..., __aligned_size, __size - __aligned_size);
            
            return _Function_<vx::scalar_tag>()(std::forward<_Args_>(__args)...);
        }

        using callable_tag_type = __impl;
    };
};

template <template <class> class _Function_, class _Type_, class _Return_>
constexpr inline auto __dispatch_sized_impl = raze::options::functor<typename _Configurable_sized_isa_dispatcher<_Function_, _Type_, _Return_>::__impl>;

__RAZE_VX_NAMESPACE_END
