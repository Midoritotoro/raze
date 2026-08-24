#pragma once 

#include <raze/vx/Simd.h>
#include <raze/options/Options.h>

__RAZE_VX_NAMESPACE_BEGIN

template <template <class> class F, class T, class Ret, arch::ISA Forced, arch::ISA ... Candidates>
struct configurable_isa_dispatcher_t {
    template <class Options>
    struct impl : options::callable<impl, Options>{
        template <class ... Args>
        raze_always_inline Ret operator()(sizetype size, Args&& ... args) const noexcept {
            return options::dispatch_call(*this, size, std::forward<Args>(args)...);
        }

        template <sizetype Size, class ... Args>
        raze_always_inline Ret operator()(std::integral_constant<sizetype, Size> size,
            Args&& ... args) const noexcept
        {
            return options::dispatch_call(*this, size, std::forward<Args>(args)...);
        }

        template <sizetype Size, class ... Args>
        static raze_always_inline Ret deferred_call(auto opts,
            std::integral_constant<sizetype, Size> size, Args&& ... args) noexcept requires(sizeof...(Candidates) == 0)
        {
            if constexpr (Forced != arch::ISA::None) {
                constexpr auto vector_size = (vx::default_width<Forced> / 8);

                if constexpr (Size < vector_size)
                    return F<vx::scalar_tag>()(std::forward<Args>(args)...);

                constexpr auto aligned_size = Size & ~sizetype(vector_size - 1);
                using V = simd<T, runtime_abi<Forced, vector_size / sizeof(T)>>;
                return F<V>()(std::integral_constant<sizetype, aligned_size>{},
                    std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
            }
            else {
                if constexpr (Size < 16)
                    return F<vx::scalar_tag>()(std::forward<Args>(args)...);

                i32 all = arch::ProcessorFeatures::all();

                if constexpr (Size >= 64) {
                    constexpr auto aligned_size = Size & ~0x3F;

                    if constexpr (sizeof(T) >= 4) {
                        using V = simd<T, runtime_abi<arch::ISA::AVX512F, 64 / sizeof(T)>>;

                        if constexpr (has_avx512f<target_isa()>) {
                            return F<V>()(std::integral_constant<sizetype, aligned_size>{},
                                std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
                        }
                        else {
                            if (arch::ProcessorFeatures::has<arch::features::AVX512F>(all)) {
                                return F<V>()(std::integral_constant<sizetype, aligned_size>{},
                                    std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
                            }
                        }
                    }
                    else {
                        using V = simd<T, runtime_abi<arch::ISA::AVX512BW, 64 / sizeof(T)>>;

                        if constexpr (has_avx512bw<target_isa()>) {
                            return F<V>()(std::integral_constant<sizetype, aligned_size>{},
                                std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
                        }
                        else {
                            if (arch::ProcessorFeatures::has<arch::features::AVX512BW>(all)) {
                                return F<V>()(std::integral_constant<sizetype, aligned_size>{},
                                    std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
                            }
                        }
                    }
                }

                if constexpr (Size >= 32) {
                    constexpr auto aligned_size = Size & ~0x1F;
                    using V = simd<T, runtime_abi<arch::ISA::AVX2, 32 / sizeof(T)>>;

                    if constexpr (has_avx2<target_isa()>) {
                        return F<V>()(std::integral_constant<sizetype, aligned_size>{},
                            std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
                    }
                    else {
                        if (arch::ProcessorFeatures::has<arch::features::AVX2>(all))
                            return F<V>()(std::integral_constant<sizetype, aligned_size>{},
                                std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
                    }
                }

                if constexpr (Size >= 16) {
                    constexpr auto aligned_size = Size & ~0xF;
                    using V = simd<T, runtime_abi<arch::ISA::SSE42, 16 / sizeof(T)>>;

                    if constexpr (has_sse42<target_isa()>) {
                        return F<V>()(std::integral_constant<sizetype, aligned_size>{},
                            std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
                    }
                    else {
                        if (arch::ProcessorFeatures::has<arch::features::SSE42>(all))
                            return F<V>()(std::integral_constant<sizetype, aligned_size>{},
                                std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
                    }

                    return F<simd<T, runtime_abi<arch::ISA::SSE2, 16 / sizeof(T)>>>()(
                        std::integral_constant<sizetype, aligned_size>{},
                        std::integral_constant<sizetype, Size - aligned_size>{}, std::forward<Args>(args)...);
                }
            }
        }

        template <class ... Args>
        static raze_always_inline Ret deferred_call(auto opts,
            sizetype size, Args&& ... args) noexcept requires(sizeof...(Candidates) == 0)
        {
           if constexpr (Forced != arch::ISA::None) {
                constexpr auto vector_size = (vx::default_width<Forced> / 8);

                if (size < vector_size)
                    return F<vx::scalar_tag>()(std::forward<_Args_>(args)...);

                const auto aligned_size = size & ~sizetype(vector_size - 1);
                using V = simd<T, runtime_abi<Forced, vector_size / sizeof(T)>>;
                return F<V>()(aligned_size, size - aligned_size, std::forward<Args>(args)...);
           }
           else {
                if (size < 16) return F<vx::scalar_tag>()(std::forward<Args>(args)...);
                const auto all = arch::ProcessorFeatures::all();

                if (size >= 64) {
                    if constexpr (sizeof(T) >= 4) {
                        if (arch::ProcessorFeatures::has<arch::features::AVX512F>(all))
                            return F<simd<T, runtime_abi<arch::ISA::AVX512F, 64 / sizeof(T)>>>()(
                                size & ~0x3F, size & 0x3F, std::forward<Args>(args)...);
                    }
                    else {
                        if (arch::ProcessorFeatures::has<arch::features::AVX512BW>(all))
                            return F<simd<T, runtime_abi<arch::ISA::AVX512BW, 64 / sizeof(T)>>>()(
                                size & ~0x3F, size & 0x3F, std::forward<Args>(args)...);
                    }
                }

                if (size >= 32 && arch::ProcessorFeatures::has<arch::features::AVX2>(all))
                    return F<simd<T, runtime_abi<arch::ISA::AVX2, 32 / sizeof(T)>>>()(
                        size & ~0x1F, size & 0x1F, std::forward<Args>(args)...);

                return F<simd<T, runtime_abi<arch::ISA::SSE2, 16 / sizeof(T)>>>()(
                    size & ~0xF, size & 0xF, std::forward<Args>(args)...);
           }
        }

        template <arch::ISA ISA, arch::ISA ... Rest, class ... Args>
        static raze_always_inline Ret try_dispatch(sizetype size, i32 all, Args&& ... args) noexcept {
            constexpr auto vector_size = vx::default_width<ISA> / 8;

            if (size >= vector_size && arch::ProcessorFeatures::has<arch::feature_of(ISA)>(all)) {
                using V = simd<T, runtime_abi<ISA, vector_size / sizeof(T)>>;
                return F<V>()(size & ~(vector_size - 1), size & (vector_size - 1), std::forward<Args>(args)...);
            }

            if constexpr (sizeof...(Rest) != 0) return try_dispatch<Rest...>(size, all, std::forward<Args>(args)...);
            else F<vx::scalar_tag>()(std::forward<Args>(args)...);
        }

        template <class ... Args>
        static raze_always_inline Ret deferred_call(auto opts,
            sizetype size, _Args_&& ... args) noexcept requires(sizeof...(Candidates) != 0)
        {
            if constexpr (_ForcedISA_ != arch::ISA::None) {
                constexpr auto vector_size = vx::default_width<Forced> / 8;

                if (size < vector_size)
                    return F<vx::scalar_tag>()(std::forward<Args>(args)...);

                using V = simd<T, runtime_abi<_ForcedISA_, vector_size / sizeof(T)>>;
                return F<V>()(size & ~sizetype(vector_size - 1), size & (vector_size - 1), std::forward<Args>(args)...);
            }
            else {
                if (size < 16) return F<vx::scalar_tag>()(std::forward<Args>(args)...);
                return try_dispatch<Candidates...>(size, arch::ProcessorFeatures::all(), std::forward<Args>(args)...);
            }
        }
    };
};

consteval arch::ISA forced_isa() noexcept {
#if defined(raze_cpp_clang) || defined(raze_cpp_gnu) || RAZE_ISA_FORCE_ENABLED
	return target_isa();
#else
    return arch::ISA::None;
#endif
}

template <template <class> class F, class T, class Ret,
    arch::ISA Forced = forced_isa(), arch::ISA ... Candidates>
static inline constexpr auto dispatch = raze::options::functor<
    configurable_isa_dispatcher_t<F, T, Ret, Forced, Candidates...>::template impl>;

__RAZE_VX_NAMESPACE_END
