#pragma once

#include <src/raze/algorithm/Autovectorizable.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/arch/CpuFeature.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#if defined(raze_cpp_msvc)
#pragma strict_gs_check(off)
#endif

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

template <arch::ISA First, arch::ISA ... Rest>
consteval arch::ISA find_best_isa() {
    if constexpr (sizeof...(Rest) == 0) return First;
    else {
        constexpr arch::ISA best_rest = find_best_isa<Rest...>();
        if constexpr (to_feature_mask<First>() >= to_feature_mask<best_rest>()) return First;
        else return best_rest;
    }
}

template <arch::ISA Target, arch::ISA First, arch::ISA ... Rest>
consteval arch::ISA find_best_supported_isa() {
    if constexpr (sizeof...(Rest) == 0) return single_satisfies<First, Target>() ? First : arch::ISA::None;
    else {
        constexpr arch::ISA rest_supported = find_best_supported_isa<Target, Rest...>();

        if constexpr (!single_satisfies<First, Target>()) return rest_supported;
        else if constexpr (rest_supported == arch::ISA::None) return First;
        else {
            if constexpr (to_feature_mask<First>() >= to_feature_mask<rest_supported>()) return First;
            else return rest_supported;
        }
    }
}

template <class T>
static consteval auto is_best_isa_default() {
    return (sizeof(T) >= 4 && vx::has_avx512f<vx::target_isa()>) || (vx::has_avx512bw<vx::target_isa()>);
}

template <arch::ISA ... Features>
class targets {
public:
    static constexpr std::array<arch::ISA, sizeof...(Features)> features = { Features... };

    template <class T, class Kernel = void>
    static consteval bool have_best_isa() {
        if constexpr (sizeof...(Features) == 0) return is_best_isa_default<T>();
        else {
            constexpr arch::ISA best_feature = find_best_isa<Features...>();
            return single_satisfies<best_feature, vx::target_isa()>();
        }
    }

    template <class T>
    static consteval arch::ISA resolve_static_isa() {
        if constexpr (sizeof...(Features) == 0) return vx::target_isa();
        else return find_best_supported_isa<vx::target_isa(), Features...>();
    }

    template <class T, class Traits, class Kernel, bool IsFstatic, arch::ISA StaticISA>
    static consteval auto make_dispatcher() {
        if constexpr (IsFstatic) return vx::dispatch<options::unroller_t<Traits>::template impl, T, StaticISA>;
        else return vx::dispatch<options::unroller_t<Traits>::template impl, T, arch::ISA::None, Features...>;
    }
};

template <class F>
class dispatchable {
    template <class Kernel>
    static consteval auto get_targets() {
        if constexpr (requires { Kernel::targets(); }) return Kernel::targets();
        else return targets<>{};
    }
public:
    template <class ... Args>
    constexpr raze_always_inline auto dispatch(Args&& ... args) const {
        auto work = typename F::kernel(std::forward<Args>(args)...);

        using Traits = decltype(static_cast<const F*>(this)->traits());
        using Kernel = decltype(work);
        using Value = typename Kernel::vector_value_type;

        if constexpr (requires { work.exit(); }&& requires { work.default_result(); }) {
            if (work.exit()) return work.default_result();
        }

        constexpr auto target_policy = get_targets<Kernel>();

        constexpr bool has_best = target_policy.template have_best_isa<Value, Kernel>();
        constexpr arch::ISA static_isa = target_policy.template resolve_static_isa<Value>();
        constexpr bool has_supported_isa = (static_isa != arch::ISA::None);

#if defined(raze_cpp_clang) || defined(raze_cpp_gnu) || defined(__clang__) || defined(__GNUC__)
        constexpr bool is_gcc_or_clang = true;
#else
        constexpr bool is_gcc_or_clang = false;
#endif

        constexpr bool is_fstatic = is_gcc_or_clang ? has_supported_isa
            : (has_best || options::is_fstatic<Traits>() || options::get_strategy<Traits>().is_manual());

        constexpr bool can_vectorize = !options::is_fscalar<Traits>() && Kernel::vectorizable() &&
            (is_gcc_or_clang ? has_supported_isa : true);

        if constexpr (can_vectorize) {
            if !consteval {
                constexpr auto dispatcher = target_policy.template make_dispatcher<Value, Traits, Kernel, is_fstatic, static_isa>();

                return dispatcher(work.size(), work);
            }
        }
#if (defined(raze_cpp_clang) && raze_cpp_clang >= 1500) || defined(raze_cpp_gnu)
        else if constexpr (!options::is_fscalar<Traits>() && autovectorizable_kernel<Kernel>
            && options::get_strategy<Traits>().is_autovec())
        {
            return invoke_scalar_autovec(work);
        }
#endif

        return options::unroller<Traits, vx::scalar_tag>(work);
    }
};

#if defined(raze_cpp_msvc)
#pragma strict_gs_check(on)
#endif

template <class T>
concept vectorizable_tag = !std::same_as<T, vx::scalar_tag>&& vx::simd_type<T>;

template <class T>
concept scalar_tag = std::same_as<T, vx::scalar_tag>;

template <class T>
concept tail_tag = requires(T) { typename T::original_type; } && !vx::simd_type<T>;

__RAZE_ALGORITHM_NAMESPACE_END