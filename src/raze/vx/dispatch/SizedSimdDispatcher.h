#pragma once 

#include <raze/vx/Simd.h>
#include <raze/options/Options.h>

__RAZE_VX_NAMESPACE_BEGIN

template <template <class> class _Function_, class _Type_, class _Return_, arch::ISA _ForcedISA_, arch::ISA ... _Candidates_>
struct _Configurable_sized_isa_dispatcher {
    template <class _Options_>
    struct __impl : raze::options::strict_elementwise_callable<__impl, _Options_>{
        template <class ... _Args_>
        raze_always_inline _Return_ operator()(sizetype __size, _Args_&& ... __args) const noexcept {
            return raze::options::__dispatch_call(*this, __size, std::forward<_Args_>(__args)...);
        }

        template <sizetype _Size_, class ... _Args_>
        raze_always_inline _Return_ operator()(std::integral_constant<sizetype, _Size_> __size,
            _Args_&& ... __args) const noexcept
        {
            return raze::options::__dispatch_call(*this, __size, std::forward<_Args_>(__args)...);
        }

        template <sizetype _Size_, class ... _Args_>
        static raze_always_inline _Return_ deferred_call(auto __options,
            std::integral_constant<sizetype, _Size_> __size, _Args_&& ... __args) noexcept requires(sizeof...(_Candidates_) == 0)
        {
            if constexpr (_ForcedISA_ != arch::ISA::None) {
                constexpr auto __vector_size = (vx::__default_width<_ForcedISA_> / 8);

                if constexpr (_Size_ < __vector_size) {
                    return _Function_<vx::scalar_tag>()(std::forward<_Args_>(__args)...);
                }

                constexpr auto __aligned_size = _Size_ & ~(__vector_size - 1);
                using _Simd_ = simd<_Type_, runtime_abi<_ForcedISA_, __vector_size / sizeof(_Type_)>>;
                return _Function_<_Simd_>()(std::integral_constant<sizetype, __aligned_size>{},
                    std::integral_constant<sizetype, _Size_ - __aligned_size>{}, std::forward<_Args_>(__args)...);
            }
            else {
                if constexpr (_Size_ < 16) {
                    return _Function_<vx::scalar_tag>()(std::forward<_Args_>(__args)...);
                }

                i32 __all;

                if constexpr (_Size_ >= 64) {
                    constexpr auto __aligned_size = _Size_ & ~0x3F;

                    if constexpr (sizeof(_Type_) >= 4) {
                        using _Simd_ = simd<_Type_, runtime_abi<arch::ISA::AVX512F, 64 / sizeof(_Type_)>>;

                        if constexpr (__has_avx512f_support_v<__best_isa_compile_time()>) {
                            return _Function_<_Simd_>()(std::integral_constant<sizetype, __aligned_size>{},
                                std::integral_constant<sizetype, _Size_ - __aligned_size>{}, std::forward<_Args_>(__args)...);
                        }
                        else {
                            __all = arch::ProcessorFeatures::all();
                            if (arch::ProcessorFeatures::has<arch::__features::AVX512F>(__all)) {
                                return _Function_<_Simd_>()(std::integral_constant<sizetype, __aligned_size>{},
                                    std::integral_constant<sizetype, _Size_ - __aligned_size>{}, std::forward<_Args_>(__args)...);
                            }
                        }
                    }
                    else {
                        using _Simd_ = simd<_Type_, runtime_abi<arch::ISA::AVX512BW, 64 / sizeof(_Type_)>>;

                        if constexpr (__has_avx512bw_support_v<__best_isa_compile_time()>) {
                            return _Function_<_Simd_>()(std::integral_constant<sizetype, __aligned_size>{},
                                std::integral_constant<sizetype, _Size_ - __aligned_size>{}, std::forward<_Args_>(__args)...);
                        }
                        else {
                            __all = arch::ProcessorFeatures::all();
                            if (arch::ProcessorFeatures::has<arch::__features::AVX512BW>(__all)) {
                                return _Function_<_Simd_>()(std::integral_constant<sizetype, __aligned_size>{},
                                    std::integral_constant<sizetype, _Size_ - __aligned_size>{}, std::forward<_Args_>(__args)...);
                            }
                        }
                    }
                }

                if constexpr (_Size_ >= 32) {
                    constexpr auto __aligned_size = _Size_ & ~0x1F;
                    using _Simd_ = simd<_Type_, runtime_abi<arch::ISA::AVX2, 32 / sizeof(_Type_)>>;

                    if constexpr (__has_avx2_support_v<__best_isa_compile_time()>) {
                        return _Function_<_Simd_>()(std::integral_constant<sizetype, __aligned_size>{},
                            std::integral_constant<sizetype, _Size_ - __aligned_size>{}, std::forward<_Args_>(__args)...);
                    }
                    else {
                        __all = arch::ProcessorFeatures::all();
                        if (arch::ProcessorFeatures::has<arch::__features::AVX2>(__all))
                            return _Function_<_Simd_>()(std::integral_constant<sizetype, __aligned_size>{},
                                std::integral_constant<sizetype, _Size_ - __aligned_size>{}, std::forward<_Args_>(__args)...);
                    }
                }

                if constexpr (_Size_ >= 16) {
                    constexpr auto __aligned_size = _Size_ & ~0xF;
                    using _Simd_ = simd<_Type_, runtime_abi<arch::ISA::SSE42, 16 / sizeof(_Type_)>>;

                    if constexpr (__has_sse42_support_v<__best_isa_compile_time()>) {
                        return _Function_<_Simd_>()(std::integral_constant<sizetype, __aligned_size>{},
                            std::integral_constant<sizetype, _Size_ - __aligned_size>{}, std::forward<_Args_>(__args)...);
                    }
                    else {
                        __all = arch::ProcessorFeatures::all();
                        if (arch::ProcessorFeatures::has<arch::__features::SSE42>(__all))
                            return _Function_<_Simd_>()(std::integral_constant<sizetype, __aligned_size>{},
                                std::integral_constant<sizetype, _Size_ - __aligned_size>{}, std::forward<_Args_>(__args)...);
                    }
                }
            }
        }

        template <class ... _Args_>
        static raze_always_inline _Return_ deferred_call(auto __options,
            sizetype __size, _Args_&& ... __args) noexcept requires(sizeof...(_Candidates_) == 0)
        {
           if constexpr (_ForcedISA_ != arch::ISA::None) {
                constexpr auto __vector_size = (vx::__default_width<_ForcedISA_> / 8);

                if (__size < __vector_size)
                    return _Function_<vx::scalar_tag>()(std::forward<_Args_>(__args)...);

                const auto __aligned_size = __size & ~(__vector_size - 1);
                using _Simd_ = simd<_Type_, runtime_abi<_ForcedISA_, __vector_size / sizeof(_Type_)>>;
                return _Function_<_Simd_>()(__aligned_size, __size - __aligned_size, std::forward<_Args_>(__args)...);
           }
           else {
                if (__size < 16) return _Function_<vx::scalar_tag>()(std::forward<_Args_>(__args)...);
                const auto __all = arch::ProcessorFeatures::all();

                /*if (__size >= 64) {
                    if constexpr (sizeof(_Type_) >= 4) {
                        if (arch::ProcessorFeatures::has<arch::__features::AVX512F>(__all))
                            return _Function_<simd<_Type_, runtime_abi<arch::ISA::AVX512F, 64 / sizeof(_Type_)>>>()(
                                __size & ~0x3F, __size & 0x3F, std::forward<_Args_>(__args)...);
                    }
                    else {
                        if (arch::ProcessorFeatures::has<arch::__features::AVX512BW>(__all))
                            return _Function_<simd<_Type_, runtime_abi<arch::ISA::AVX512BW, 64 / sizeof(_Type_)>>>()(
                                __size & ~0x3F, __size & 0x3F, std::forward<_Args_>(__args)...);
                    }
                }*/

                if (__size >= 32 && arch::ProcessorFeatures::has<arch::__features::AVX2>(__all))
                    return _Function_<simd<_Type_, runtime_abi<arch::ISA::AVX2, 32 / sizeof(_Type_)>>>()(
                        __size & ~0x1F, __size & 0x1F, std::forward<_Args_>(__args)...);

                return _Function_<simd<_Type_, runtime_abi<arch::ISA::SSE2, 16 / sizeof(_Type_)>>>()(
                    __size & ~0xF, __size & 0xF, std::forward<_Args_>(__args)...);
           }
        }

        template <arch::ISA _ISA_, arch::ISA ... _Rest_, class ... _Args_>
        static raze_always_inline _Return_ __try_dispatch(sizetype __size, i32 __all, _Args_&& ... __args) noexcept {
            constexpr auto __vector_size = vx::__default_width<_ISA_> / 8;

            if (__size >= __vector_size && arch::ProcessorFeatures::has<arch::__feature_of(_ISA_)>(__all)) {
                using _Simd_ = simd<_Type_, runtime_abi<_ISA_, __vector_size / sizeof(_Type_)>>;
                return _Function_<_Simd_>()(__size & ~(__vector_size - 1), __size & (__vector_size - 1), std::forward<_Args_>(__args)...);
            }

            if constexpr (sizeof...(_Rest_) != 0) return __try_dispatch<_Rest_...>(__size, __all, std::forward<_Args_>(__args)...);
            else _Function_<vx::scalar_tag>()(std::forward<_Args_>(__args)...);
        }

        template <class ... _Args_>
        static raze_always_inline _Return_ deferred_call(auto __options,
            sizetype __size, _Args_&& ... __args) noexcept requires(sizeof...(_Candidates_) != 0)
        {
            if constexpr (_ForcedISA_ != arch::ISA::None) {
                constexpr auto __vector_size = vx::__default_width<_ForcedISA_> / 8;

                if (__size < __vector_size)
                    return _Function_<vx::scalar_tag>()(std::forward<_Args_>(__args)...);

                using _Simd_ = simd<_Type_, runtime_abi<_ForcedISA_, __vector_size / sizeof(_Type_)>>;
                return _Function_<_Simd_>()(__size & ~(__vector_size - 1), __size & (__vector_size - 1), std::forward<_Args_>(__args)...);
            }
            else {
                if (__size < 16) return _Function_<vx::scalar_tag>()(std::forward<_Args_>(__args)...);
                return __try_dispatch<_Candidates_...>(__size, arch::ProcessorFeatures::all(), std::forward<_Args_>(__args)...);
            }
        }

        using callable_tag_type = __impl;
    };
};

template <template <class> class _Function_, class _Type_, class _Return_,
    arch::ISA _ForcedISA_ = arch::ISA::None, arch::ISA ... _Candidates_>
static inline constexpr auto __dispatch_sized_impl = raze::options::functor<typename
    _Configurable_sized_isa_dispatcher<_Function_, _Type_, _Return_, _ForcedISA_, _Candidates_...>::__impl>;

__RAZE_VX_NAMESPACE_END
