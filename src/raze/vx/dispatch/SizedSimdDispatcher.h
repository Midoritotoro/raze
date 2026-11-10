#pragma once

#include <raze/vx/Simd.h>
#include <raze/options/Options.h>
#include <raze/arch/ProcessorFeatures.h>
#include <src/raze/vx/hw/x86/memory/ConditionalMemAccessNative.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA>
consteval u32 to_feature_mask() {
    constexpr u32 f_none = 0;
    constexpr u32 f_sse = 1u << 1;
    constexpr u32 f_sse2 = (1u << 2) | f_sse;
    constexpr u32 f_sse3 = (1u << 3) | f_sse2;
    constexpr u32 f_ssse3 = (1u << 4) | f_sse3;
    constexpr u32 f_sse41 = (1u << 5) | f_ssse3;
    constexpr u32 f_sse42 = (1u << 6) | f_sse41;
    constexpr u32 f_avx = (1u << 7) | f_sse42;
    constexpr u32 f_fma3 = (1u << 8) | f_avx;
    constexpr u32 f_avx2 = (1u << 9) | f_avx;
    constexpr u32 f_avx512f = (1u << 10) | f_avx2;
    constexpr u32 f_avx512bw = (1u << 11) | f_avx512f;
    constexpr u32 f_avx512dq = (1u << 12) | f_avx512f;
    constexpr u32 f_avx512vl = (1u << 13) | f_avx512f;
    constexpr u32 f_avx512vbmi = (1u << 14) | f_avx512bw;
    constexpr u32 f_avx512vbmi2 = (1u << 15) | f_avx512bw;

    if constexpr (ISA == arch::ISA::None)            return f_none;
    else if constexpr (ISA == arch::ISA::SSE)             return f_sse;
    else if constexpr (ISA == arch::ISA::SSE2)            return f_sse2;
    else if constexpr (ISA == arch::ISA::SSE3)            return f_sse3;
    else if constexpr (ISA == arch::ISA::SSSE3)           return f_ssse3;
    else if constexpr (ISA == arch::ISA::SSE41)           return f_sse41;
    else if constexpr (ISA == arch::ISA::SSE42)           return f_sse42;
    else if constexpr (ISA == arch::ISA::AVX)             return f_avx;
    else if constexpr (ISA == arch::ISA::FMA3)            return f_fma3;
    else if constexpr (ISA == arch::ISA::AVX2)            return f_avx2;
    else if constexpr (ISA == arch::ISA::AVX2FMA3)        return f_avx2 | f_fma3;
    else if constexpr (ISA == arch::ISA::AVX512F)         return f_avx512f;
    else if constexpr (ISA == arch::ISA::AVX512BW)        return f_avx512bw;
    else if constexpr (ISA == arch::ISA::AVX512DQ)        return f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512BWDQ)      return f_avx512bw | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VLF)       return f_avx512vl;
    else if constexpr (ISA == arch::ISA::AVX512VLBW)      return f_avx512vl | f_avx512bw;
    else if constexpr (ISA == arch::ISA::AVX512VLDQ)      return f_avx512vl | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VLBWDQ)    return f_avx512vl | f_avx512bw | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VBMI)      return f_avx512vbmi;
    else if constexpr (ISA == arch::ISA::AVX512VBMI2)     return f_avx512vbmi2;
    else if constexpr (ISA == arch::ISA::AVX512VBMIVL)    return f_avx512vbmi | f_avx512vl;
    else if constexpr (ISA == arch::ISA::AVX512VBMI2VL)   return f_avx512vbmi2 | f_avx512vl;
    else if constexpr (ISA == arch::ISA::AVX512VBMIDQ)    return f_avx512vbmi | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VBMI2DQ)   return f_avx512vbmi2 | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VBMIVLDQ)  return f_avx512vbmi | f_avx512vl | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VBMI2VLDQ) return f_avx512vbmi2 | f_avx512vl | f_avx512dq;
    else return 0;
}

template <arch::ISA Required, arch::ISA Target>
consteval bool single_satisfies() {
    if constexpr (Required == arch::ISA::None) return true;
    if constexpr (Target == arch::ISA::None)   return false;

    constexpr auto target_m = to_feature_mask<Target>();
    constexpr auto required_m = to_feature_mask<Required>();
    if constexpr (required_m == 0) return false;
    return (target_m & required_m) == required_m;
}

template <template <class> class F, class T, arch::ISA Forced, arch::ISA ... Candidates>
struct configurable_isa_dispatcher_t {
    template <class Options>
    struct impl : options::callable<impl, Options> {
        template <class Work>
        raze_always_inline decltype(auto) operator()(sizetype size, Work& work) const {
            return options::dispatch_call(*this, size, work);
        }

        template <sizetype Size, class Work>
        raze_always_inline decltype(auto) operator()(std::integral_constant<sizetype, Size> size,
            Work& work) const
        {
            return options::dispatch_call(*this, size, work);
        }

        static constexpr arch::ISA avx512_isa() noexcept {
            return sizeof(T) >= 4 ? arch::ISA::AVX512F : arch::ISA::AVX512BW;
        }

        static constexpr bool compile_has_avx512() noexcept {
            return sizeof(T) >= 4 ? has_avx512f<target_isa()> : has_avx512bw<target_isa()>;
        }

        static constexpr bool compile_has_avx512vl() noexcept {
            return has_avx512vl<target_isa()>;
        }

        static raze_always_inline bool runtime_has_avx512(i32 all) noexcept {
            if constexpr (sizeof(T) >= 4) return arch::ProcessorFeatures::has<arch::features::AVX512F>(all);
            else return arch::ProcessorFeatures::has<arch::features::AVX512BW>(all);
        }

        static raze_always_inline arch::ISA detect_best_isa(i32 all) noexcept {
            if constexpr (compile_has_avx512())
                return avx512_isa();
            if (runtime_has_avx512(all))
                return avx512_isa();

            if constexpr (has_avx2<target_isa()>)
                return arch::ISA::AVX2;
            if (arch::ProcessorFeatures::has<arch::features::AVX2>(all))
                return arch::ISA::AVX2;

            if constexpr (has_sse42<target_isa()>)
                return arch::ISA::SSE42;
            if (arch::ProcessorFeatures::has<arch::features::SSE42>(all))
                return arch::ISA::SSE42;

            return arch::ISA::SSE2;
        }

        static raze_always_inline arch::ISA detect_best_isa_at_most_avx2(i32 all) noexcept {
            if constexpr (has_avx2<target_isa()>)
                return arch::ISA::AVX2;
            if (arch::ProcessorFeatures::has<arch::features::AVX2>(all) || runtime_has_avx512(all))
                return arch::ISA::AVX2;

            if constexpr (has_sse42<target_isa()>)
                return arch::ISA::SSE42;
            if (arch::ProcessorFeatures::has<arch::features::SSE42>(all))
                return arch::ISA::SSE42;

            return arch::ISA::SSE2;
        }

        template <arch::ISA ISA, sizetype VecBytes, class Work>
        static constexpr bool has_working_simd_for = requires {
            std::declval<Work&>()(
                simd<T, runtime_abi<ISA, VecBytes / sizeof(T)>>{ uninitialized },
                sizetype{}
                );
        };

#if defined(raze_cpp_clang) || defined(raze_cpp_gnu) || defined(raze_cpp_icc)
        template <arch::ISA ISA, class Work>
        static constexpr bool has_working_simd = single_satisfies<ISA, target_isa()> () &&
            has_working_simd_for<ISA, vx::default_width<ISA> / 8, Work>;
#else
        template <arch::ISA ISA, class Work>
        static constexpr bool has_working_simd = has_working_simd_for<ISA, vx::default_width<ISA> / 8, Work>;
#endif

        template <arch::ISA ISA, sizetype VecBytes, class Work>
        static raze_always_inline decltype(auto) call_dyn(sizetype size, Work& work) {
            using V = simd<T, runtime_abi<ISA, VecBytes / sizeof(T)>>;
            return F<V>()(size & ~sizetype(VecBytes - 1),
                size & sizetype(VecBytes - 1), work);
        }

        template <arch::ISA ISA, sizetype VecBytes, sizetype Size, class Work>
        static raze_always_inline decltype(auto) call_const(Work& work) {
            using V = simd<T, runtime_abi<ISA, VecBytes / sizeof(T)>>;
            constexpr auto aligned_sz = Size & ~sizetype(VecBytes - 1);
            return F<V>()(std::integral_constant<sizetype, aligned_sz>{},
                std::integral_constant<sizetype, Size - aligned_sz>{}, work);
        }

        template <arch::ISA ISA, sizetype VecBytes, class Work>
        static raze_always_inline decltype(auto) call_tail(sizetype size, Work& work) {
            using V = simd<T, runtime_abi<ISA, VecBytes / sizeof(T)>>;
            return F<V>()(size, work);
        }

        template <sizetype Size, class Work>
        static raze_always_inline decltype(auto) deferred_call(auto,
            std::integral_constant<sizetype, Size> size, Work& work)
            requires(sizeof...(Candidates) == 0)
        {
            if constexpr (Forced != arch::ISA::None) {
                constexpr auto vector_size = vx::default_width<Forced> / 8;
                if constexpr (Size < vector_size) return F<vx::scalar_tag>()(work);
                else if constexpr (has_working_simd<Forced, Work>)
                    return call_const<Forced, vector_size, Size>(work);
                else return F<vx::scalar_tag>()(work);
            }
            else if constexpr (Size < 16) {
                return F<vx::scalar_tag>()(work);
            }
            else if constexpr (Size < 32) {
                if constexpr (!has_working_simd<arch::ISA::SSE2, Work>)
                    return F<vx::scalar_tag>()(work);
                else if constexpr (has_sse42<target_isa()>)
                    return call_const<arch::ISA::SSE42, 16, Size>(work);
                else
                    return call_const<arch::ISA::SSE2, 16, Size>(work);
            }
            else if constexpr (Size < 64) {
                if constexpr (has_avx2<target_isa()> && has_working_simd<arch::ISA::AVX2, Work>)
                    return call_const<arch::ISA::AVX2, 32, Size>(work);
                else if constexpr (has_working_simd<arch::ISA::SSE2, Work>) {
                    if constexpr (has_sse42<target_isa()>)
                        return call_const<arch::ISA::SSE42, 16, Size>(work);
                    else
                        return call_const<arch::ISA::SSE2, 16, Size>(work);
                }
                else return F<vx::scalar_tag>()(work);
            }
            else if constexpr (compile_has_avx512() && has_working_simd<avx512_isa(), Work>) {
                return call_const<avx512_isa(), 64, Size>(work);
            }
            else {
                const i32 all = arch::ProcessorFeatures::all();
                switch (detect_best_isa(all)) {
                case arch::ISA::AVX512F:
                case arch::ISA::AVX512BW:
                    if constexpr (has_working_simd<avx512_isa(), Work>)
                        return call_const<avx512_isa(), 64, Size>(work);
                    [[fallthrough]];
                case arch::ISA::AVX2:
                    if constexpr (has_working_simd<arch::ISA::AVX2, Work>)
                        return call_const<arch::ISA::AVX2, 32, Size>(work);
                    [[fallthrough]];
                case arch::ISA::SSE42:
                    if constexpr (has_working_simd<arch::ISA::SSE42, Work>)
                        return call_const<arch::ISA::SSE42, 16, Size>(work);
                    [[fallthrough]];
                default:
                    if constexpr (has_working_simd<arch::ISA::SSE2, Work>)
                        return call_const<arch::ISA::SSE2, 16, Size>(work);
                    break;
                }
                return F<vx::scalar_tag>()(work);
            }
        }

        template <class Work>
        static raze_always_inline decltype(auto) deferred_call(auto, sizetype size, Work& work)
            requires(sizeof...(Candidates) == 0)
        {
            if constexpr (Forced != arch::ISA::None) {
                constexpr auto vector_size = vx::default_width<Forced> / 8;
                if constexpr (has_working_simd<Forced, Work>) {
                    if (size >= vector_size) return call_dyn<Forced, vector_size>(size, work);
                    if constexpr (native_conditional_memory_access<Forced, T>)
                        return call_tail<Forced, vector_size>(size, work);
                }
                return F<vx::scalar_tag>()(work);
            }
            else
#if defined(raze_cpp_msvc_only)
                if constexpr (compile_has_avx512() && has_working_simd<avx512_isa(), Work>) {
                    if (size >= 64) return call_dyn<target_isa(), 64>(size, work);
                    if constexpr (native_conditional_memory_access<target_isa(), T>)
                        return call_tail<target_isa(), 64>(size, work);
                }
                else if constexpr (has_avx2<target_isa()>) {
                    const i32 all = arch::ProcessorFeatures::all();

                    if (runtime_has_avx512(all)) {
                        if constexpr (has_working_simd<avx512_isa(), Work>) {
                            if (size >= 64) return call_dyn<avx512_isa(), 64>(size, work);
                            if constexpr (native_conditional_memory_access<avx512_isa(), T>)
                                return call_tail<avx512_isa(), 64>(size, work);
                        }
                    }

                    if constexpr (has_working_simd<arch::ISA::AVX2, Work>) {
                        if (size >= 32) return call_dyn<arch::ISA::AVX2, 32>(size, work);
                        if constexpr (native_conditional_memory_access<arch::ISA::AVX2, T>)
                            return call_tail<arch::ISA::AVX2, 32>(size, work);
                    }
                }
                else {
                    const i32 all = arch::ProcessorFeatures::all();

                    switch (detect_best_isa(all)) {
                    case arch::ISA::AVX512F:
                    case arch::ISA::AVX512BW:
                        if constexpr (has_working_simd<avx512_isa(), Work>)
                            if (size >= 64) return call_dyn<avx512_isa(), 64>(size, work);
                        [[fallthrough]];
                    case arch::ISA::AVX2:
                        if constexpr (has_working_simd<arch::ISA::AVX2, Work>)
                            if (size >= 32) return call_dyn<arch::ISA::AVX2, 32>(size, work);
                        [[fallthrough]];
                    case arch::ISA::SSE42:
                        if constexpr (has_working_simd<arch::ISA::SSE42, Work>)
                            if (size >= 16) return call_dyn<arch::ISA::SSE42, 16>(size, work);
                        break;
                    case arch::ISA::SSE2:
                        if constexpr (has_working_simd<arch::ISA::SSE2, Work>)
                            if (size >= 16) return call_dyn<arch::ISA::SSE2, 16>(size, work);
                        [[fallthrough]];
                    default:
                        break;
                    }
                }
#else
                if constexpr (compile_has_avx512() && has_working_simd<avx512_isa(), Work>) {
                    if (size >= 64) return call_dyn<target_isa(), 64>(size, work);
                    if constexpr (native_conditional_memory_access<target_isa(), T>)
                        return call_tail<target_isa(), 64>(size, work);
                }
                else if constexpr (has_avx2<target_isa()> && has_working_simd<arch::ISA::AVX2, Work>) {
                    if (size >= 32) return call_dyn<target_isa(), 32>(size, work);
                    if constexpr (native_conditional_memory_access<target_isa(), T>)
                        return call_tail<target_isa(), 32>(size, work);
                }
                else if constexpr (has_working_simd<arch::ISA::SSE2, Work>) {
                    if (size >= 16) return call_dyn<target_isa(), 16>(size, work);
                }
#endif

            return F<vx::scalar_tag>()(work);
        }

        template <arch::ISA ISA, arch::ISA ... Rest, class Work>
        static raze_always_inline decltype(auto) try_dispatch(sizetype size, i32 all, Work& work) {
            if constexpr (!has_working_simd<ISA, Work>) {
                if constexpr (sizeof...(Rest) != 0) return try_dispatch<Rest...>(size, all, work);
                else return F<vx::scalar_tag>()(work);
            }
            else {
                constexpr auto vector_size = vx::default_width<ISA> / 8;
                if (size >= vector_size && arch::ProcessorFeatures::has<arch::feature_of(ISA)>(all))
                    return call_dyn<ISA, vector_size>(size, work);

                if constexpr (sizeof...(Rest) != 0) return try_dispatch<Rest...>(size, all, work);
                else return F<vx::scalar_tag>()(work);
            }
        }

        template <class Work>
        static raze_always_inline decltype(auto) deferred_call(auto, sizetype size, Work& work)
            requires(sizeof...(Candidates) != 0)
        {
            if constexpr (Forced != arch::ISA::None) {
                constexpr auto vector_size = vx::default_width<Forced> / 8;
                if constexpr (has_working_simd<Forced, Work>) {
                    if (size >= vector_size) return call_dyn<Forced, vector_size>(size, work);
                }
                return F<vx::scalar_tag>()(work);
            }
            else {
                if (size < 16)
                    return F<vx::scalar_tag>()(work);
                return try_dispatch<Candidates...>(size, arch::ProcessorFeatures::all(), work);
            }
        }
    };
};

consteval raze_always_inline arch::ISA forced_isa() noexcept {
#if defined(raze_cpp_clang) || defined(raze_cpp_gnu)
    return target_isa();
#else
    if constexpr (static_cast<int>(target_isa()) == static_cast<int>(arch::ISA::SSE2)) return arch::ISA::None;
    else return target_isa();
#endif
}

template <template <class> class F, class T,
    arch::ISA Forced = forced_isa(), arch::ISA ... Candidates>
static inline constexpr auto dispatch = raze::options::functor<
    configurable_isa_dispatcher_t<F, T, Forced, Candidates...>::template impl>;

__RAZE_VX_NAMESPACE_END