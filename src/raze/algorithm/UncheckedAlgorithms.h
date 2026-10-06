#pragma once

#include <src/raze/algorithm/Autovectorizable.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/arch/CpuFeature.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#pragma strict_gs_check(off)

template <class Function, arch::ISA ... Other>
struct dispatchable {
    template <class ... Args>
    constexpr raze_always_inline auto dispatch(Args&& ... args) const {
        auto work = typename Function::kernel(std::forward<Args>(args)...);

        using TraitsType = decltype(static_cast<const Function*>(this)->traits());
        using WorkType = decltype(work);
        using Value = typename WorkType::vector_value_type;

        if constexpr (requires { work.exit(); } && requires { work.default_result(); }) {
            if (work.exit()) return work.default_result();
        }

        constexpr auto have_best_isa = vx::has_avx512bw<vx::target_isa()> ||
            (vx::has_avx512f<vx::target_isa()> && sizeof(Value) >= 4);

        if constexpr ((!options::is_fscalar<TraitsType>() &&
            WorkType::vectorizable() && (options::get_strategy<TraitsType>().is_manual())
            || (have_best_isa && WorkType::vectorizable() && options::get_strategy<TraitsType>().is_autovec()))
            || (options::is_fstatic<TraitsType>() && WorkType::vectorizable()))
        {
            if !consteval {
                constexpr auto get_dispatcher = []() constexpr {
                    if constexpr (options::is_fstatic<TraitsType>()) return vx::dispatch<options::unroller_t<TraitsType>::template impl, Value, vx::target_isa()>;
                    else return vx::dispatch<options::unroller_t<TraitsType>::template impl, Value, /* Forced */ arch::ISA::None, Other...>;
                };

                constexpr auto dispatcher = get_dispatcher();

                if constexpr (requires { WorkType::static_size(); }) return dispatcher(WorkType::static_size(), work);
                else return dispatcher(work.size(), work);
            }
        }
#if (defined(raze_cpp_clang) && raze_cpp_clang >= 1500) || defined(raze_cpp_gnu)
        else if constexpr (!options::is_fscalar<TraitsType>() && autovectorizable_kernel<WorkType>
            && options::get_strategy<TraitsType>().is_autovec())
        {
            return invoke_scalar_autovec(work);
        }
#endif
        
        return options::unroller<TraitsType, vx::scalar_tag>(work);
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
