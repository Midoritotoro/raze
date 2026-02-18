#pragma once 

#include <raze/datapar/Simd.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <template <class> class _Function_>
struct __simd_dispatcher {
private:
    template <
        class       _SpecializedFunction_,
        class...    _Args_>
    raze_always_inline static auto __invoke_simd_helper(_Args_&&... __args) noexcept {
        return _SpecializedFunction_()(std::forward<_Args_>(__args)...);
    }

    template <
        class       _SpecializedFunction_,
        class...    _VectorizedArgs_>
    raze_always_inline static auto __invoke_simd(std::tuple<_VectorizedArgs_...>&& __simd_args) noexcept {
        return std::apply(&__invoke_simd_helper<_SpecializedFunction_, _VectorizedArgs_...>, std::move(__simd_args));
    }
public:
    template <
        class       _Type_,
        class       _FallbackFunction_,
        class ...   _VectorizedArgs_,
        class ...   _FallbackArgs_>
    raze_always_inline static auto __apply(
        _FallbackFunction_&&            __fallback,
        std::tuple<_VectorizedArgs_...> __simd_args,
        std::tuple<_FallbackArgs_...>   __fallback_args) noexcept
    {
#if RAZE_ISA_FORCE_ENABLED
        return __invoke_simd<_Function_<simd_forced<_Type_>>>(std::forward<_Args_>(__args)...);
#else
        if (arch::ProcessorFeatures::AVX512BW()) {
            if (arch::ProcessorFeatures::AVX512DQ())
                return _Function_<simd512_avx512bwdq<_Type_>>()(std::move(__simd_args));

            return __invoke_simd<_Function_<simd512_avx512bw<_Type_>>>(std::move(__simd_args));
        }
        else if (arch::ProcessorFeatures::AVX512F()) {
            if (arch::ProcessorFeatures::AVX512DQ())
                return __invoke_simd<_Function_<simd512_avx512dq<_Type_>>>(std::move(__simd_args));

            return __invoke_simd<_Function_<simd512_avx512f<_Type_>>>(std::move(__simd_args));
        }
        
        else if (arch::ProcessorFeatures::AVX2()) {
            return __invoke_simd<_Function_<simd256_avx2<_Type_>>>(std::move(__simd_args));
        }

        else if (arch::ProcessorFeatures::SSE2()) {
            return __invoke_simd<_Function_<simd128_sse2<_Type_>>>(std::move(__simd_args));
        }

        else {
            return std::apply(type_traits::__pass_function(__fallback), std::move(__fallback_args));
        }
#endif
    }

    template <
        class       _Type_,
        class       _FallbackFunction_,
        class ...   _Args_>
    raze_always_inline static auto __apply(
        _FallbackFunction_&&    __fallback,
        _Args_...               __args) noexcept
    {
#if RAZE_ISA_FORCE_ENABLED
        return _Function_<simd_forced<_Type_>>()(std::forward<_Args_>(__args)...);
#else
        if (arch::ProcessorFeatures::AVX512BW()) {
            if (arch::ProcessorFeatures::AVX512DQ())
                return _Function_<simd512_avx512bwdq<_Type_>>()(std::forward<_Args_>(__args)...);

            return _Function_<simd512_avx512bw<_Type_>>()(std::forward<_Args_>(__args)...);
        }
        else if (arch::ProcessorFeatures::AVX512F()) {
            if (arch::ProcessorFeatures::AVX512DQ())
                return _Function_<simd512_avx512dq<_Type_>>()(std::forward<_Args_>(__args)...);

            return _Function_<simd512_avx512f<_Type_>>()(std::forward<_Args_>(__args)...);
        }

        else if (arch::ProcessorFeatures::AVX2()) {
            return _Function_<simd256_avx2<_Type_>>()(std::forward<_Args_>(__args)...);
        }

        else if (arch::ProcessorFeatures::SSE2()) {
            return _Function_<simd128_sse2<_Type_>>()(std::forward<_Args_>(__args)...);
        }

        else {
            return type_traits::invoke(type_traits::__pass_function(__fallback), std::forward<_Args_>(__args)...);
        }
#endif
    }
};

__RAZE_DATAPAR_NAMESPACE_END
