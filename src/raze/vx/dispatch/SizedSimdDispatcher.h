#pragma once 

#include <raze/vx/Simd.h>
#include <raze/options/Options.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Fn_, class _Tuple_>
raze_always_inline decltype(auto) unpack(_Fn_&& __fn, _Tuple_&& __tuple) noexcept {
    return std::apply(std::forward<_Fn_>(__fn), std::forward<_Tuple_>(__tuple));
}

template <class _Fn_, class _Tuple_, class... _Suffix_>
raze_always_inline decltype(auto) unpack_suffix(_Fn_&& __fn, _Tuple_&& __tuple, _Suffix_&&... __suffix) noexcept {
    return std::apply([&] <class... _E_> (_E_&&... __elements) -> decltype(auto) {
        return std::forward<_Fn_>(__fn)(std::forward<_E_>(__elements)..., std::forward<_Suffix_>(__suffix)...);
    }, std::forward<_Tuple_>(__tuple));
}

template <template <class> class _Function_, class _Type_, class _Return_>
struct _Configurable_sized_isa_dispatcher {
    template <class _Options_>
    struct __impl : raze::options::strict_elementwise_callable<__impl, _Options_>{
        template <class _Size_, class _FallbackArgsTuple_, class _SimdArgsTuple_>
        raze_always_inline _Return_ operator()(_Size_ __size, _FallbackArgsTuple_ __fb_tup,
            _SimdArgsTuple_ __simd_tup) const noexcept
        {
            return raze::options::__dispatch_call(*this, __size, __fb_tup, __simd_tup);
        }

        template <class _Size_, class _FallbackArgsTuple_, class _SimdArgsTuple_>
        static raze_always_inline _Return_ deferred_call(auto __options, _Size_ __size,
            _FallbackArgsTuple_ __fb_tup, _SimdArgsTuple_ __simd_tup) noexcept
        {
            /*if (const auto __aligned_size = __size & ~0x3F; __aligned_size != 0) {
                if (arch::ProcessorFeatures::AVX512BW()) {
                    return _Function_<simd<_Type_, runtime_abi<arch::ISA::AVX512BW, 64 / sizeof(_Type_)>>>()(
                        __aligned_size, __size & 64 - sizeof(_Type_), std::forward<_Args_>(__args)...);
                }
                else if (arch::ProcessorFeatures::AVX512F()) {
                    return _Function_<simd<_Type_, runtime_abi<arch::ISA::AVX512F, 64 / sizeof(_Type_)>>>()(
                        __aligned_size, __size & 64 - sizeof(_Type_), std::forward<_Args_>(__args)...);
                }
            }

            if (const auto __aligned_size = __size & ~0x1F;  __aligned_size != 0) {
                return _Function_<simd<_Type_, runtime_abi<arch::ISA::AVX2, 32 / sizeof(_Type_)>>>()(__aligned_size, __size & (32) -
                    sizeof(_Type_), std::forward<_Args_>(__args)...);
            }

            else*/ if (const auto __aligned_size = __size & ~0xF; __aligned_size != 0 && arch::ProcessorFeatures::SSE42()) {
                return unpack_suffix(_Function_<simd<_Type_, runtime_abi<arch::ISA::SSE42, 16 / sizeof(_Type_)>>>(), __simd_tup,
                    __aligned_size, __size & 16 - sizeof(_Type_));
            }
            
            return unpack(_Function_<vx::scalar_tag>(), __fb_tup);
        }

        using callable_tag_type = __impl;
    };
};

template <template <class> class _Function_, class _Type_, class _Return_>
constexpr inline auto __dispatch_sized_impl = raze::options::functor<typename _Configurable_sized_isa_dispatcher<_Function_, _Type_, _Return_>::__impl>;

__RAZE_VX_NAMESPACE_END
