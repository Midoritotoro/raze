#pragma once 

#include <raze/datapar/Simd.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    template <class> class  _Callable_,
    class                   _Type_>
class __simd_dispatcher {
public:
    template <
        class       _FallbackFunction_,
        class ...   _Args_>
    raze_static_operator raze_always_inline auto operator()(
        _FallbackFunction_&&    __fallback,
        _Args_...               __args) raze_const_operator noexcept
    {
#if RAZE_ISA_FORCE_ENABLED
        return _Function_<simd_forced<_Type_>>()(std::forward<_Args_>(__args)...);
#else
        if (arch::ProcessorFeatures::AVX512F()) {
            if (arch::ProcessorFeatures::AVX512BW()) {
                if (arch::ProcessorFeatures::AVX512DQ()) {
                    return _Callable_<simd512_avx512bwdq<_Type_>>()(std::forward<_Args_>(__args)...);
                }
                return _Callable_<simd512_avx512bw<_Type_>>()(std::forward<_Args_>(__args)...);
            }

            return _Callable_<simd512_avx512f<_Type_>>()(std::forward<_Args_>(__args)...);
        }

        else if (arch::ProcessorFeatures::AVX2()) {
            return _Callable_<simd256_avx2<_Type_>>()(std::forward<_Args_>(__args)...);
        }

        else if (arch::ProcessorFeatures::SSE42()) {
            return _Callable_<simd128_sse42<_Type_>>()(std::forward<_Args_>(__args)...);
        }

        else if (arch::ProcessorFeatures::SSE2()) {
            return _Callable_<simd128_sse2<_Type_>>()(std::forward<_Args_>(__args)...);
        }

        else {
            return type_traits::invoke(type_traits::__pass_function(__fallback), std::forward<_Args_>(__args)...);
        }
#endif
    }
};

__RAZE_DATAPAR_NAMESPACE_END
