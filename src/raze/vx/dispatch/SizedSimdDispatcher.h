#pragma once 

#include <raze/vx/Simd.h>
#include <raze/options/Options.h>

__RAZE_VX_NAMESPACE_BEGIN

template <template <class> class _Function_, class _Type_>
struct _Configurable_sized_isa_dispatcher {
    template <class _Options_>
    struct __impl : raze::options::callable<__impl, _Options_> {
        template <class _Size_, class _Fallback_, class ... _Args_>
        raze_static_operator raze_always_inline auto operator()(_Size_ __size,
            _Fallback_&& __fb, _Args_... __args) raze_const_operator noexcept
                -> decltype(std::forward<_Fallback_>(__fb)(__args...))
        {
            return raze::options::__dispatch_call(*this, __size, std::forward<_Fallback_>(__fb), __args...);
        }

        template <class _Size_, class _Fallback_, class ... _Args_>
        static raze_always_inline auto deferred_call(_Size_ __size, _Fallback_&& __fb, _Args_... __args) noexcept {
            if (const auto __aligned_size = __size & ~0x3F; __aligned_size != 0) {
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

            else if (const auto __aligned_size = __size & ~0xF; __aligned_size != 0 && arch::ProcessorFeatures::SSE42()) {
                return _Function_<simd<_Type_, runtime_abi<arch::ISA::SSE42, 16 / sizeof(_Type_)>>>()(
                    __aligned_size, __size & 16 - sizeof(_Type_), std::forward<_Args_>(__args)...);
            }

            return std::invoke(type_traits::__pass_function(__fb), std::forward<_Args_>(__args)...);
        }
    };
};

template <template <class> class _Function_, class _Type_>
constexpr inline auto __dispatch_sized_impl = raze::options::functor<typename _Configurable_sized_isa_dispatcher<_Function_, _Type_>::__impl>;

__RAZE_VX_NAMESPACE_END
