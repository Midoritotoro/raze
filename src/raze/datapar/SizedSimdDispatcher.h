#pragma once 

#include <raze/datapar/Simd.h>

__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    template <class> class  _Function_,
    class                   _Type_>
class __simd_sized_dispatcher {
public:
    template <
        class       _SizeType_,
        class       _FallbackFunction_,
        class ...   _Args_>
    raze_static_operator raze_always_inline auto operator()(
        _SizeType_              __size,
        _FallbackFunction_&&    __fallback,
        _Args_...               __args) raze_const_operator noexcept
    {
#if RAZE_ISA_FORCE_ENABLED
        return _Function_<simd_forced<_Type_>>()(
            __size & (~(sizeof(simd_forced<_Type_>) - 1)), __size & (sizeof(simd_forced<_Type_>) -
            sizeof(typename simd_forced<_Type_>::value_type)), std::forward<_Args_>(__args)...);
#else
        if (const auto __aligned_size = __size & (~(sizeof(simd512_avx512bw<_Type_>) - 1)); __aligned_size != 0) {
            if (arch::ProcessorFeatures::AVX512BW()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return _Function_<simd512_avx512bwdq<_Type_>>()(
                        __aligned_size, __size & (sizeof(simd512_avx512bwdq<_Type_>) -
                        sizeof(typename simd512_avx512bwdq<_Type_>::value_type)),
                        std::forward<_Args_>(__args)...);

                return _Function_<simd512_avx512bw<_Type_>>()(
                    __aligned_size, __size & (sizeof(simd512_avx512bw<_Type_>) -
                    sizeof(typename simd512_avx512bw<_Type_>::value_type)),
                    std::forward<_Args_>(__args)...);
            }
            else if (arch::ProcessorFeatures::AVX512F()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return _Function_<simd512_avx512dq<_Type_>>()(
                        __aligned_size, __size & (sizeof(simd512_avx512dq<_Type_>) -
                        sizeof(typename simd512_avx512dq<_Type_>::value_type)),
                        std::forward<_Args_>(__args)...);

                return _Function_<simd512_avx512f<_Type_>>()(
                    __aligned_size, __size & (sizeof(simd512_avx512f<_Type_>) -
                    sizeof(typename simd512_avx512f<_Type_>::value_type)), 
                    std::forward<_Args_>(__args)...);
            }
        }

        if (const auto __aligned_size = __size & (~(sizeof(simd256_avx2<_Type_>) - 1)); 
            __aligned_size != 0 && arch::ProcessorFeatures::AVX2()) 
        {
            return _Function_<simd256_avx2<_Type_>>()(
                __aligned_size, __size & (sizeof(simd256_avx2<_Type_>) -
                sizeof(typename simd256_avx2<_Type_>::value_type)),
                std::forward<_Args_>(__args)...);
        }

        else if (const auto __aligned_size = __size & (~(sizeof(simd128_sse42<_Type_>) - 1));
            __aligned_size != 0 && arch::ProcessorFeatures::SSE42())
        {
            return _Function_<simd128_sse42<_Type_>>()(
                __aligned_size, __size & (sizeof(simd128_sse42<_Type_>) - 
                sizeof(typename simd128_sse42<_Type_>::value_type)),
                std::forward<_Args_>(__args)...);
        }

        return type_traits::invoke(
            type_traits::__pass_function(__fallback), std::forward<_Args_>(__args)...);
#endif
    }
};

__RAZE_DATAPAR_NAMESPACE_END
