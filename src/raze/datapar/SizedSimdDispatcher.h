#pragma once 

#include <raze/datapar/Simd.h>

__RAZE_DATAPAR_NAMESPACE_BEGIN

template <template <class> class _Function_>
struct __simd_sized_dispatcher {
private:
    template <
        class _Simd_,
        class _SizeType_>
    raze_always_inline static std::pair<_SizeType_, _SizeType_> __sizes(_SizeType_ __size) noexcept {
        return { 
            __size & (~(sizeof(_Simd_) - 1)),
            __size & (sizeof(_Simd_) - sizeof(typename _Simd_::value_type)) 
        };
    }

    template <
        class       _SpecializedFunction_, 
        class       _SizeType_,
        class...    _Args>
    raze_always_inline static auto __invoke_simd_helper(
        _SizeType_ __aligned_size,
        _SizeType_ __tail_size,
        _Args&&... __args) noexcept
    {
        return _SpecializedFunction_()(__aligned_size, __tail_size, std::forward<_Args>(__args)...);
    }

    template <
        class       _SpecializedFunction_,
        class       _SizeType_,
        class...    _VectorizedArgs_>
    raze_always_inline static auto __invoke_simd(
        _SizeType_                          __aligned_size,
        _SizeType_                          __tail_size,
        std::tuple<_VectorizedArgs_...>&&   __simd_args) noexcept
    {
        return std::apply(&__invoke_simd_helper<_SpecializedFunction_, _SizeType_, _VectorizedArgs_...>,
            std::tuple_cat(std::make_tuple(__aligned_size, __tail_size), std::move(__simd_args)));
    }
public:
    template <
        class       _Type_,
        class       _SizeType_,
        class       _FallbackFunction_,
        class ...   _VectorizedArgs_,
        class ...   _FallbackArgs_>
    raze_always_inline static auto __apply(
        _SizeType_                      __size,
        _FallbackFunction_&&            __fallback,
        std::tuple<_VectorizedArgs_...> __simd_args,
        std::tuple<_FallbackArgs_...>   __fallback_args) noexcept
    {
#if RAZE_ISA_FORCE_ENABLED
        if (__size < sizeof(simd_forced<_Type_>))
            return std::apply(type_traits::__pass_function(__fallback), std::move(__fallback_args));

        else if (const auto __aligned_size = __size & (~(sizeof(simd_forced<_Type_>) - 1)); __aligned_size != 0)
            return __invoke_simd<_Function_<simd_forced<_Type_>>>(__aligned_size, __size & (sizeof(simd_forced<_Type_>) -
                sizeof(typename simd_forced<_Type_>::value_type)), std::move(__simd_args));
#else 
        if (__size < 16)
            return std::apply(type_traits::__pass_function(__fallback), std::move(__fallback_args));

        else if (const auto __aligned_size = __size & (~(sizeof(simd512_avx512bw<_Type_>) - 1)); __aligned_size != 0) {
            if (arch::ProcessorFeatures::AVX512BW()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return __invoke_simd<_Function_<simd512_avx512bwdq<_Type_>>>(__aligned_size, __size & (sizeof(simd512_avx512bwdq<_Type_>) -
                        sizeof(typename simd512_avx512bwdq<_Type_>::value_type)), std::move(__simd_args));

                return __invoke_simd<_Function_<simd512_avx512bw<_Type_>>>(__aligned_size, __size & (sizeof(simd512_avx512bw<_Type_>) -
                    sizeof(typename simd512_avx512bw<_Type_>::value_type)), std::move(__simd_args));
            }
            else if (arch::ProcessorFeatures::AVX512F()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return __invoke_simd<_Function_<simd512_avx512dq<_Type_>>>(__aligned_size, __size & (sizeof(simd512_avx512dq<_Type_>) -
                        sizeof(typename simd512_avx512dq<_Type_>::value_type)), std::move(__simd_args));

                return __invoke_simd<_Function_<simd512_avx512f<_Type_>>>(__aligned_size, __size & (sizeof(simd512_avx512f<_Type_>) -
                    sizeof(typename simd512_avx512f<_Type_>::value_type)), std::move(__simd_args));
            }
        }

        else if (const auto __aligned_size = __size & (~(sizeof(simd256_avx512vlbw<_Type_>) - 1)); __aligned_size != 0) {
            if (arch::ProcessorFeatures::AVX512BW() && arch::ProcessorFeatures::AVX512VL()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return __invoke_simd<_Function_<simd256_avx512vlbwdq<_Type_>>>(__aligned_size, __size & (sizeof(simd256_avx512vlbwdq<_Type_>) -
                        sizeof(typename simd256_avx512vlbwdq<_Type_>::value_type)), std::move(__simd_args));

                return __invoke_simd<_Function_<simd256_avx512vlbw<_Type_>>>(__aligned_size, __size & (sizeof(simd256_avx512vlbw<_Type_>) -
                    sizeof(typename simd256_avx512vlbw<_Type_>::value_type)), std::move(__simd_args));
            }
            else if (arch::ProcessorFeatures::AVX512F() && arch::ProcessorFeatures::AVX512VL()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return __invoke_simd<_Function_<simd256_avx512vldq<_Type_>>>(__aligned_size, __size & (sizeof(simd256_avx512vldq<_Type_>) -
                        sizeof(typename simd256_avx512vldq<_Type_>::value_type)), std::move(__simd_args));

                return __invoke_simd<_Function_<simd256_avx512vlf<_Type_>>>(__aligned_size, __size & (sizeof(simd256_avx512vlf<_Type_>) -
                    sizeof(typename simd256_avx512vlf<_Type_>::value_type)), std::move(__simd_args));
            }
        }
        
        else if (const auto __aligned_size = __size & (~(sizeof(simd256_avx2<_Type_>) - 1)); __aligned_size != 0 && arch::ProcessorFeatures::AVX2()) {
            return __invoke_simd<_Function_<simd256_avx2<_Type_>>>(__aligned_size, __size & (sizeof(simd256_avx2<_Type_>) -
                sizeof(typename simd256_avx2<_Type_>::value_type)), std::move(__simd_args));
        }

        else if (const auto __aligned_size = __size & (~(sizeof(simd128_sse2<_Type_>) - 1)); __aligned_size != 0 && arch::ProcessorFeatures::SSE2()) {
            return __invoke_simd<_Function_<simd128_sse2<_Type_>>>(__aligned_size, __size & (sizeof(simd128_sse2<_Type_>) - 
                sizeof(typename simd128_sse2<_Type_>::value_type)), std::move(__simd_args));
        }
#endif
    }

    template <
        class       _Type_,
        class       _SizeType_,
        class       _FallbackFunction_,
        class ...   _Args_>
    raze_always_inline static auto __apply(
        _SizeType_              __size,
        _FallbackFunction_&&    __fallback,
        _Args_...               __args) noexcept
    {
#if RAZE_ISA_FORCE_ENABLED
        if (__size < sizeof(simd_forced<_Type_>))
            return type_traits::invoke(type_traits::__pass_function(__fallback), std::forward<_Args_>(__args)...);

        else if (const auto __aligned_size = __size & (~(sizeof(simd_forced<_Type_>) - 1)); __aligned_size != 0)
            return _Function_<simd_forced<_Type_>>()(__aligned_size, __size & (sizeof(simd_forced<_Type_>) -
                sizeof(typename simd_forced<_Type_>::value_type)), std::forward<_Args_>(__args)...);
#else
        if (__size < 16)
            return type_traits::invoke(type_traits::__pass_function(__fallback), std::forward<_Args_>(__args)...);
        else if (const auto __aligned_size = __size & (~(sizeof(simd512_avx512bw<_Type_>) - 1)); __aligned_size != 0) {
            if (arch::ProcessorFeatures::AVX512BW()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return _Function_<simd512_avx512bwdq<_Type_>>()(__aligned_size, __size & (sizeof(simd512_avx512bwdq<_Type_>) -
                        sizeof(typename simd512_avx512bwdq<_Type_>::value_type)), std::forward<_Args_>(__args)...);

                return _Function_<simd512_avx512bw<_Type_>>()(__aligned_size, __size & (sizeof(simd512_avx512bw<_Type_>) -
                    sizeof(typename simd512_avx512bw<_Type_>::value_type)), std::forward<_Args_>(__args)...);
            }
            else if (arch::ProcessorFeatures::AVX512F()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return _Function_<simd512_avx512dq<_Type_>>()(__aligned_size, __size & (sizeof(simd512_avx512dq<_Type_>) -
                        sizeof(typename simd512_avx512dq<_Type_>::value_type)), std::forward<_Args_>(__args)...);

                return _Function_<simd512_avx512f<_Type_>>()(__aligned_size, __size & (sizeof(simd512_avx512f<_Type_>) -
                    sizeof(typename simd512_avx512f<_Type_>::value_type)), std::forward<_Args_>(__args)...);
            }
        }

        else if (const auto __aligned_size = __size & (~(sizeof(simd256_avx512vlbw<_Type_>) - 1)); __aligned_size != 0) {
            if (arch::ProcessorFeatures::AVX512BW() && arch::ProcessorFeatures::AVX512VL()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return _Function_<simd256_avx512vlbwdq<_Type_>>()(__aligned_size, __size & (sizeof(simd256_avx512vlbwdq<_Type_>) -
                        sizeof(typename simd256_avx512vlbwdq<_Type_>::value_type)), std::forward<_Args_>(__args)...);

                return _Function_<simd256_avx512vlbw<_Type_>>()(__aligned_size, __size & (sizeof(simd256_avx512vlbw<_Type_>) -
                    sizeof(typename simd256_avx512vlbw<_Type_>::value_type)), std::forward<_Args_>(__args)...);
            }
            else if (arch::ProcessorFeatures::AVX512F() && arch::ProcessorFeatures::AVX512VL()) {
                if (arch::ProcessorFeatures::AVX512DQ())
                    return _Function_<simd256_avx512vldq<_Type_>>()(__aligned_size, __size & (sizeof(simd256_avx512vldq<_Type_>) -
                        sizeof(typename simd256_avx512vldq<_Type_>::value_type)), std::forward<_Args_>(__args)...);

                return _Function_<simd256_avx512vlf<_Type_>>()(__aligned_size, __size & (sizeof(simd256_avx512vlf<_Type_>) -
                    sizeof(typename simd256_avx512vlf<_Type_>::value_type)), std::forward<_Args_>(__args)...);
            }
        }
        
        else if (const auto __aligned_size = __size & (~(sizeof(simd256_avx2<_Type_>) - 1)); __aligned_size != 0 && arch::ProcessorFeatures::AVX2()) {
            return _Function_<simd256_avx2<_Type_>>()(__aligned_size, __size & (sizeof(simd256_avx2<_Type_>) -
                sizeof(typename simd256_avx2<_Type_>::value_type)), std::forward<_Args_>(__args)...);
        }

        else if (const auto __aligned_size = __size & (~(sizeof(simd128_sse2<_Type_>) - 1)); __aligned_size != 0 && arch::ProcessorFeatures::SSE2()) {
            return _Function_<simd128_sse2<_Type_>>()(__aligned_size, __size & (sizeof(simd128_sse2<_Type_>) - 
                sizeof(typename simd128_sse2<_Type_>::value_type)), std::forward<_Args_>(__args)...);
        }
#endif
    }
};

__RAZE_DATAPAR_NAMESPACE_END
