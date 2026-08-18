#pragma once

#include <raze/options/Options.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

enum class strategy_mode : u8 {
    manual,
    autovec,
    scalar
};

template <strategy_mode _GCC_ = strategy_mode::manual,
    strategy_mode _Clang_ = strategy_mode::manual,
    strategy_mode _MSVC_ = strategy_mode::manual>
struct strategy {
    static constexpr strategy_mode gcc = _GCC_;
    static constexpr strategy_mode clang = _Clang_;
    static constexpr strategy_mode msvc = _MSVC_;

    static constexpr strategy_mode effective() noexcept {
#if defined(raze_cpp_gnu)
        return _GCC_;
#elif defined(raze_cpp_clang)
        return _Clang_;
#elif defined(raze_cpp_msvc)
        return _MSVC_;
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

    template <strategy_mode _M_>
    constexpr auto for_gcc() const noexcept {
        return strategy<_M_, _Clang_, _MSVC_>{};
    }

    template <strategy_mode _M_>
    constexpr auto for_clang() const noexcept {
        return strategy<_GCC_, _M_, _MSVC_>{};
    }

    template <strategy_mode _M_>
    constexpr auto for_msvc() const noexcept {
        return strategy<_GCC_, _Clang_, _M_>{};
    }

    template <strategy_mode _M_>
    constexpr auto for_all() const noexcept {
        return strategy<_M_, _M_, _M_>{};
    }
};

__RAZE_ALGORITHM_NAMESPACE_END