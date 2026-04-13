#pragma once

#include <src/raze/type_traits/Invoke.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

struct accumulate_decorations {
    inline constexpr auto operator()(auto __accumulator, const auto& __m) const { 
        return __m.default_to(__accumulator); 
    }
};

template <
    class _Function_,
    class _Type_> 
struct foldable {
    _Function_ _function;
    _Type_ _value;

    template <class _OtherType_>
    friend constexpr raze_always_inline auto operator>>(
        foldable&&                          __x, 
        foldable<_Function_, _OtherType_>&& __y) noexcept
    {
        using _ReturnType = decltype(raze::type_traits::invoke(__x._function, __x._value, __y._value));
        return foldable<_Function_, std::decay_t<_ReturnType>>{__x._function,
            raze::type_traits::invoke(__x._function, __x._value, __y._value)};
    }

    template <class _OtherType_>
    friend constexpr raze_always_inline decltype(auto) operator<<(
        foldable&&                          __x, 
        foldable<_Function_, _OtherType_>&& __y) noexcept 
    {
        using _ReturnType = decltype(raze::type_traits::invoke(__x._function, __x._value, __y._value));
        return foldable<_Function_, std::decay_t<_ReturnType>>{__x._function,
            raze::type_traits::invoke(__x._function, __x._value, __y._value)};
    }
};

template <
    class _Function_,
    class _Type_>
foldable(const _Function_&, _Type_&&) -> foldable<_Function_, _Type_>;

template <
    class _Function_, 
    class _Type_,
    class _Value_>
constexpr raze_always_inline auto fold_left(
    _Function_&&    __function, 
    _Type_&&        __tuple, 
    _Value_         __init) noexcept 
{
    return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return (foldable{ __function, __init } >> ... >> foldable{
            __function, std::get<I>(std::forward<_Type_>(__tuple))})._value;
    }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<_Type_>>>());
}

__RAZE_OPTIONS_NAMESPACE_END
