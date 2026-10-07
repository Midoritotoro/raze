#pragma once

#include <src/raze/algorithm/Autovectorizable.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/arch/CpuFeature.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#pragma strict_gs_check(off)

template <arch::ISA ISA>
consteval u32 to_feature_mask() {
    constexpr u32 f_none = 1u << 0;
    constexpr u32 f_sse = 1u << 1;
    constexpr u32 f_sse2 = (1u << 2) | f_sse;
    constexpr u32 f_sse3 = (1u << 3) | f_sse2;
    constexpr u32 f_ssse3 = (1u << 4) | f_sse3;
    constexpr u32 f_sse41 = (1u << 5) | f_ssse3;
    constexpr u32 f_sse42 = (1u << 6) | f_sse41;
    constexpr u32 f_avx = (1u << 7) | f_sse42;
    constexpr u32 f_fma3 = (1u << 8);
    constexpr u32 f_avx2 = (1u << 9) | f_avx;
    constexpr u32 f_avx512f = (1u << 10) | f_avx2;
    constexpr u32 f_avx512bw = (1u << 11) | f_avx512f;
    constexpr u32 f_avx512dq = (1u << 12) | f_avx512f;
    constexpr u32 f_avx512vl = (1u << 13) | f_avx512f;
    constexpr u32 f_avx512vbmi = (1u << 14) | f_avx512bw;
    constexpr u32 f_avx512vbmi2 = (1u << 15) | f_avx512bw;

    if constexpr (ISA == arch::ISA::None) return f_none;
    else if constexpr (ISA == arch::ISA::SSE) return f_sse;
    else if constexpr (ISA == arch::ISA::SSE2) return f_sse2;
    else if constexpr (ISA == arch::ISA::SSE3) return f_sse3;
    else if constexpr (ISA == arch::ISA::SSSE3) return f_ssse3;
    else if constexpr (ISA == arch::ISA::SSE41) return f_sse41;
    else if constexpr (ISA == arch::ISA::SSE42) return f_sse42;
    else if constexpr (ISA == arch::ISA::AVX) return f_avx;
    else if constexpr (ISA == arch::ISA::FMA3) return f_fma3;
    else if constexpr (ISA == arch::ISA::AVX2) return f_avx2;
    else if constexpr (ISA == arch::ISA::AVX2FMA3) return f_avx2 | f_fma3;
    else if constexpr (ISA == arch::ISA::AVX512F) return f_avx512f;
    else if constexpr (ISA == arch::ISA::AVX512BW) return f_avx512bw;
    else if constexpr (ISA == arch::ISA::AVX512DQ) return f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512BWDQ) return f_avx512bw | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VLF) return f_avx512vl;
    else if constexpr (ISA == arch::ISA::AVX512VLBW) return f_avx512vl | f_avx512bw;
    else if constexpr (ISA == arch::ISA::AVX512VLDQ) return f_avx512vl | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VLBWDQ) return f_avx512vl | f_avx512bw | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VBMI) return f_avx512vbmi;
    else if constexpr (ISA == arch::ISA::AVX512VBMI2) return f_avx512vbmi2;
    else if constexpr (ISA == arch::ISA::AVX512VBMIVL) return f_avx512vbmi | f_avx512vl;
    else if constexpr (ISA == arch::ISA::AVX512VBMI2VL) return f_avx512vbmi2 | f_avx512vl;
    else if constexpr (ISA == arch::ISA::AVX512VBMIDQ) return f_avx512vbmi | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VBMI2DQ) return f_avx512vbmi2 | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VBMIVLDQ) return f_avx512vbmi | f_avx512vl | f_avx512dq;
    else if constexpr (ISA == arch::ISA::AVX512VBMI2VLDQ) return f_avx512vbmi2 | f_avx512vl | f_avx512dq;
    else return 0;
}

template <arch::ISA Required, arch::ISA Target>
consteval auto single_satisfies() {
    constexpr auto target_m = to_feature_mask<Target>();
    constexpr auto required_m = to_feature_mask<Required>();
    return (target_m & required_m) == required_m;
}

template <arch::ISA Required, arch::ISA Candidate, class	Enable = void>
struct satisfies_helper : std::false_type
{};

template <arch::ISA Required, arch::ISA Candidate>
struct satisfies_helper<Required, Candidate, std::enable_if_t<single_satisfies<Required, Candidate>()>> :
    std::true_type
{};

template <arch::ISA Required, arch::ISA ... List>
struct satisfies_t {
    static constexpr bool value = (satisfies_helper<Required, List>::value || ...);
};

template <class T>
static consteval auto is_best_isa_default() {
    return (sizeof(T) >= 4 && vx::has_avx512f<vx::target_isa()>) || (vx::has_avx512bw<vx::target_isa()>);
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

template <arch::ISA ... Features>
struct targets {
    static constexpr std::array<arch::ISA, sizeof...(Features)> features = { Features... };

    template <class T, class Traits, class Kernel, bool IsFstatic>
    static consteval auto make_dispatcher() {
        if constexpr (IsFstatic) return vx::dispatch<options::unroller_t<Traits>::template impl, T, vx::target_isa()>;
        else return vx::dispatch<options::unroller_t<Traits>::template impl, T, arch::ISA::None, Features...>;
    }

    template <class T>
    static consteval bool have_best_isa() {
        if constexpr (sizeof...(Features) == 0) {
            return is_best_isa_default<T>();
        }
        else {
            constexpr arch::ISA best_feature = find_best_isa<Features...>();
            return single_satisfies<best_feature, vx::target_isa()>();
        }
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

        if constexpr (requires { work.exit(); } && requires { work.default_result(); }) {
            if (work.exit()) return work.default_result();
        }

        constexpr auto targets = get_targets<Kernel>();

        constexpr auto is_fstatic = targets.template have_best_isa<Value>() || options::is_fstatic<Traits>()
#if defined(raze_cpp_clang) || defined(raze_cpp_gnu)
            || options::get_strategy<Traits>().is_manual()
#endif
            ;

        if constexpr ((!options::is_fscalar<Traits>() && Kernel::vectorizable()) &&
            (options::get_strategy<Traits>().is_manual() || is_fstatic))
        {
            if !consteval {
                constexpr auto dispatcher = targets.template make_dispatcher<Value, Traits, Kernel, is_fstatic>();

                if constexpr (requires { Kernel::static_size(); }) return dispatcher(Kernel::static_size(), work);
                else return dispatcher(work.size(), work);
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

#pragma strict_gs_check(on)

template <class T>
concept vectorizable_tag = !std::same_as<T, vx::scalar_tag>&& vx::simd_type<T>;

template <class T>
concept scalar_tag = std::same_as<T, vx::scalar_tag>;

template <class T>
concept tail_tag = requires(T) { typename T::original_type; } && !vx::simd_type<T>;

__RAZE_ALGORITHM_NAMESPACE_END
