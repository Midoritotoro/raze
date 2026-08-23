#pragma once

#include <raze/options/Options.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

enum class strategy_mode : u8 {
    manual,
    autovec,
    scalar
};

template <strategy_mode GCC = strategy_mode::manual,
    strategy_mode Clang = strategy_mode::manual,
    strategy_mode MSVC = strategy_mode::manual>
struct strategy {
    static constexpr strategy_mode gcc = GCC;
    static constexpr strategy_mode clang = Clang;
    static constexpr strategy_mode msvc = MSVC;

    static constexpr strategy_mode effective() noexcept {
#if defined(raze_cpp_gnu)
        return GCC;
#elif defined(raze_cpp_clang)
        return Clang;
#elif defined(raze_cpp_msvc)
        return MSVC;
#else
        return strategy_mode::manual;
#endif
    }

    static constexpr bool is_autovec() noexcept {
        return effective() == strategy_mode::autovec;
    }

    static constexpr bool is_scalar() noexcept {
        return effective() == strategy_mode::scalar;
    }

    static constexpr bool is_manual() noexcept {
        return effective() == strategy_mode::manual;
    }

    template <strategy_mode M>
    constexpr auto for_gcc() const noexcept {
        return strategy<M, Clang, MSVC>{};
    }

    template <strategy_mode M>
    constexpr auto for_clang() const noexcept {
        return strategy<GCC, M, MSVC>{};
    }

    template <strategy_mode M>
    constexpr auto for_msvc() const noexcept {
        return strategy<GCC, Clang, M>{};
    }

    template <strategy_mode M>
    constexpr auto for_all() const noexcept {
        return strategy<M, M, M>{};
    }
};

__RAZE_ALGORITHM_NAMESPACE_END