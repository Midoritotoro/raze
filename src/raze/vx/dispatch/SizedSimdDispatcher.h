#pragma once

#include <raze/vx/Simd.h>
#include <raze/options/Options.h>

__RAZE_VX_NAMESPACE_BEGIN

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
        static raze_always_inline decltype(auto) call_dyn(sizetype size, Work& work) {
            using V = simd<T, runtime_abi<ISA, VecBytes / sizeof(T)>>;
            return F<V>()(size & ~sizetype(VecBytes - 1),
                          size &  sizetype(VecBytes - 1), work);
        }

        template <arch::ISA ISA, sizetype VecBytes, sizetype Size, class Work>
        static raze_always_inline decltype(auto) call_const(Work& work) {
            using V = simd<T, runtime_abi<ISA, VecBytes / sizeof(T)>>;
            constexpr auto aligned = Size & ~sizetype(VecBytes - 1);
            return F<V>()(std::integral_constant<sizetype, aligned>{},
                          std::integral_constant<sizetype, Size - aligned>{}, work);
        }

        template <sizetype Size, class Work>
        static raze_always_inline decltype(auto) deferred_call(auto,
            std::integral_constant<sizetype, Size> size, Work& work)
            requires(sizeof...(Candidates) == 0)
        {
            if constexpr (Forced != arch::ISA::None) {
                constexpr auto vector_size = vx::default_width<Forced> / 8;
                if constexpr (Size < vector_size)
                    return F<vx::scalar_tag>()(work);
                return call_const<Forced, vector_size, Size>(work);
            }
            else if constexpr (Size < 16) {
                return F<vx::scalar_tag>()(work);
            }
            else if constexpr (Size < 32) {
                if constexpr (has_sse42<target_isa()>) return call_const<arch::ISA::SSE42, 16, Size>(work);
                else return call_const<arch::ISA::SSE2, 16, Size>(work);
            }
            else if constexpr (Size < 64) {
                if constexpr (has_avx2<target_isa()>) {
                    return call_const<arch::ISA::AVX2, 32, Size>(work);
                }
                else {
                    const i32 all = arch::ProcessorFeatures::all();
                    switch (detect_best_isa_at_most_avx2(all)) {
                        case arch::ISA::AVX2:
                            return call_const<arch::ISA::AVX2, 32, Size>(work);
                        case arch::ISA::SSE42:
                            return call_const<arch::ISA::SSE42, 16, Size>(work);
                        default:
                            return call_const<arch::ISA::SSE2, 16, Size>(work);
                    }
                }
            }
            else if constexpr (compile_has_avx512()) {
                return call_const<avx512_isa(), 64, Size>(work);
            }
            else {
                const i32 all = arch::ProcessorFeatures::all();
                switch (detect_best_isa(all)) {
                    case arch::ISA::AVX512F:
                    case arch::ISA::AVX512BW:
                        return call_const<avx512_isa(), 64, Size>(work);
                    case arch::ISA::AVX2:
                        return call_const<arch::ISA::AVX2, 32, Size>(work);
                    case arch::ISA::SSE42:
                        return call_const<arch::ISA::SSE42, 16, Size>(work);
                    default:
                        return call_const<arch::ISA::SSE2, 16, Size>(work);
                }
            }
        }

        template <class Work>
        static raze_always_inline decltype(auto) deferred_call(auto, sizetype size, Work& work)
            requires(sizeof...(Candidates) == 0)
        {
            if constexpr (Forced != arch::ISA::None) {
                constexpr auto vector_size = vx::default_width<Forced> / 8;
                if (size < vector_size) return F<vx::scalar_tag>()(work);
                return call_dyn<Forced, vector_size>(size, work);
            }

            if constexpr (compile_has_avx512()) {
                if (size >= 64) return call_dyn<avx512_isa(), 64>(size, work);
                if (size >= 32) return call_dyn<arch::ISA::AVX2, 32>(size, work);
                if (size >= 16) return call_dyn<arch::ISA::SSE42, 16>(size, work);
            }
            else if constexpr (has_avx2<target_isa()>) {
                const i32 all = arch::ProcessorFeatures::all();

                if (runtime_has_avx512(all)) {
                    if (size >= 64) return call_dyn<avx512_isa(), 64>(size, work);
                    if (size >= 32) return call_dyn<arch::ISA::AVX2, 32>(size, work);
                    if (size >= 16) return call_dyn<arch::ISA::SSE42, 16>(size, work);
                    return F<vx::scalar_tag>()(work);
                }

                if (size >= 32) return call_dyn<arch::ISA::AVX2, 32>(size, work);
                if (size >= 16) return call_dyn<arch::ISA::SSE42, 16>(size, work);
            }
            else {
                const i32 all = arch::ProcessorFeatures::all();

                switch (detect_best_isa(all)) {
                    case arch::ISA::AVX512F:
                    case arch::ISA::AVX512BW:
                        if (size >= 64) return call_dyn<avx512_isa(), 64>(size, work);
                        [[fallthrough]];
                    case arch::ISA::AVX2:
                        if (size >= 32) return call_dyn<arch::ISA::AVX2, 32>(size, work);
                        [[fallthrough]];
                    case arch::ISA::SSE42:
                        if (size >= 16) return call_dyn<arch::ISA::SSE42, 16>(size, work);
                        break;
                    case arch::ISA::SSE2:
                        if (size >= 16) return call_dyn<arch::ISA::SSE2, 16>(size, work);
                        [[fallthrough]];
                    default:
                        break;
                }
            }

            return F<vx::scalar_tag>()(work);
        }

        template <arch::ISA ISA, arch::ISA ... Rest, class Work>
        static raze_always_inline decltype(auto) try_dispatch(sizetype size, i32 all, Work& work) {
            constexpr auto vector_size = vx::default_width<ISA> / 8;

            if (size >= vector_size && arch::ProcessorFeatures::has<arch::feature_of(ISA)>(all))
                return call_dyn<ISA, vector_size>(size, work);

            if constexpr (sizeof...(Rest) != 0)
                return try_dispatch<Rest...>(size, all, work);
            else
                return F<vx::scalar_tag>()(work);
        }

        template <class Work>
        static raze_always_inline decltype(auto) deferred_call(auto, sizetype size, Work& work)
            requires(sizeof...(Candidates) != 0)
        {
            if constexpr (Forced != arch::ISA::None) {
                constexpr auto vector_size = vx::default_width<Forced> / 8;
                if (size < vector_size)
                    return F<vx::scalar_tag>()(work);
                return call_dyn<Forced, vector_size>(size, work);
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