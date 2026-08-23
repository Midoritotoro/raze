#pragma once

#include <src/raze/traits/TypeTraits.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

struct accumulate_decorations {
    raze_always_inline constexpr auto operator()(auto acc, const auto& m) const {
        return m.default_to(acc); 
    }
};

template <class F, class T> 
struct foldable {
    F _function;
    T _value;

    template <class T2>
    friend constexpr raze_always_inline auto operator>>(foldable&& x,  foldable<F, T2>&& y) noexcept {
        using ReturnType = decltype(std::invoke(x._function, x._value, y._value));
        return foldable<F, std::decay_t<ReturnType>>{x._function,
            std::invoke(x._function, x._value, y._value)};
    }
};

template <class F, class T>
foldable(const F&, T&&) -> foldable<F, T>;

template <class F, class T, class Value>
constexpr raze_always_inline auto fold_left(F&& f, T&& tuple, Value init) noexcept {
    return [&] <std::size_t... I> (std::index_sequence<I...>) {
        return (foldable{ f, init } >> ... >> foldable {
            f, std::get<I>(std::forward<T>(tuple))})._value;
    }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>());
}

__RAZE_OPTIONS_NAMESPACE_END
