#pragma once 

#include <raze/vx/Simd.h>

__RAZE_VX_NAMESPACE_BEGIN

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
       if (const auto __aligned_size = __size & ~0x3F; __aligned_size != 0) {
            if (arch::ProcessorFeatures::AVX512BW()) {
                return _Function_<simd<_Type_, runtime_abi<arch::ISA::AVX512BW, 64 / sizeof(_Type_)>>>()(
                    __aligned_size, __size & 64 - sizeof(_Type_),  std::forward<_Args_>(__args)...);
            }
            else if (arch::ProcessorFeatures::AVX512F()) {
                return _Function_<simd<_Type_, runtime_abi<arch::ISA::AVX512F, 64 / sizeof(_Type_)>>>()(
                    __aligned_size, __size & 64 - sizeof(_Type_),  std::forward<_Args_>(__args)...);
            }
        }

        if (const auto __aligned_size = __size & ~0x1F;  __aligned_size != 0 && arch::ProcessorFeatures::AVX2()) {
            return _Function_<simd<_Type_, runtime_abi<arch::ISA::AVX2, 32 / sizeof(_Type_)>>>()(__aligned_size, __size & (32) -
                sizeof(_Type_), std::forward<_Args_>(__args)...);
        }

        else if (const auto __aligned_size = __size & ~0xF; __aligned_size != 0 && arch::ProcessorFeatures::SSE42()) {
            return _Function_<simd<_Type_, runtime_abi<arch::ISA::SSE42, 16 / sizeof(_Type_)>>>()(
                __aligned_size, __size & 16 - sizeof(_Type_), std::forward<_Args_>(__args)...);
        }

        return std::invoke(type_traits::__pass_function(__fallback), std::forward<_Args_>(__args)...);
#endif
    }
};

__RAZE_VX_NAMESPACE_END
