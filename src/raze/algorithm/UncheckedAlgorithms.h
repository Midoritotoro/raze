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

template <arch::ISA First, arch::ISA ... Rest>
consteval arch::ISA find_best_isa() {
    if constexpr (sizeof...(Rest) == 0) return First;
    else {
        constexpr arch::ISA best_rest = find_best_isa<Rest...>();
        if constexpr (vx::to_feature_mask<First>() >= vx::to_feature_mask<best_rest>()) return First;
        else return best_rest;
    }
}

template <arch::ISA Target, arch::ISA First, arch::ISA ... Rest>
consteval arch::ISA find_best_supported_isa() {
    if constexpr (sizeof...(Rest) == 0) return vx::single_satisfies<First, Target>() ? First : arch::ISA::None;
    else {
        constexpr arch::ISA rest_supported = find_best_supported_isa<Target, Rest...>();

        if constexpr (!vx::single_satisfies<First, Target>()) return rest_supported;
        else if constexpr (rest_supported == arch::ISA::None) return First;
        else {
            if constexpr (vx::to_feature_mask<First>() >= vx::to_feature_mask<rest_supported>()) return First;
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
        if constexpr (std::same_as<T, void>) return false;
        else if constexpr (sizeof...(Features) == 0) return is_best_isa_default<T>();
        else {
            constexpr arch::ISA best_feature = find_best_isa<Features...>();
            return vx::single_satisfies<best_feature, vx::target_isa()>();
        }
    }

    template <class T>
    static consteval arch::ISA resolve_static_isa() {
        if constexpr (std::same_as<T, void>) return arch::ISA::None;
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

        if constexpr (requires { work.expects(); } && raze::is_debug) work.expects();

        if !consteval {
            constexpr bool can_use_native = requires { work(native_tag{}); } && []() constexpr {
                if constexpr (requires { Kernel::native(); }) return Kernel::native();
                else return false;
            };
            if constexpr (can_use_native) {
                work(native_tag{});
                return work.result();
            }
        }

        constexpr auto target_policy = get_targets<Kernel>();
        constexpr bool has_best = target_policy.template have_best_isa<Value, Kernel>();

#if defined(raze_cpp_clang) || defined(raze_cpp_gnu)
        constexpr bool is_gcc_or_clang = true;
#else
        constexpr bool is_gcc_or_clang = false;
#endif

#if (defined(raze_cpp_clang) && raze_cpp_clang >= 1500 && RAZE_HAS_CLANG_RT_BUILTINS) || (defined(raze_cpp_gnu) && !defined(raze_os_windows))
        constexpr auto can_auto_vectorize = !options::is_fscalar<Traits>() && autovectorizable_kernel<Kernel>
            && options::get_strategy<Traits>().is_autovec();
#else
        // GCC doesn`t support function multiversioning on windows
        constexpr auto can_auto_vectorize = false;
#endif

        constexpr bool can_manual_vectorize = !options::is_fscalar<Traits>() && Kernel::vectorizable() &&
            (has_best || !can_auto_vectorize);

        if constexpr (can_manual_vectorize) {
            if !consteval {
                constexpr arch::ISA static_isa = target_policy.template resolve_static_isa<Value>();
                constexpr bool has_supported_isa = (static_isa != arch::ISA::None);
                constexpr bool is_fstatic = (is_gcc_or_clang && has_supported_isa && (
                    !options::get_strategy<Traits>().is_autovec() || !autovectorizable_kernel<Kernel>))
                    || (has_best || (options::is_fstatic<Traits>() && options::get_strategy<Traits>().is_manual()));

                constexpr auto dispatcher = target_policy.template make_dispatcher<Value, Traits, Kernel, is_fstatic, static_isa>();
                return dispatcher(work.size(), work);
            }
        }
        else if constexpr (can_auto_vectorize) return invoke_scalar_autovec(work);

        return options::unroller<Traits, vx::scalar_tag>(work);
    }
};

#if defined(raze_cpp_msvc)
#pragma strict_gs_check(on)
#endif

template <class T>
concept vectorizable_tag = !std::same_as<T, vx::scalar_tag> && vx::simd_type<T>;

template <class T>
concept scalar_tag = std::same_as<T, vx::scalar_tag>;

template <class T>
concept tail_tag = requires(T) { typename T::original_type; } && !vx::simd_type<T>;

__RAZE_ALGORITHM_NAMESPACE_END
