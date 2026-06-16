#pragma once 

#include <raze/compatibility/Compatibility.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
struct equal_to {
    _Type_ _v;

    constexpr explicit equal_to(_Type_ __v) noexcept: 
        _v(std::move(__v))
    {}

    template <class _Other_>
    raze_always_inline constexpr auto operator()(const _Other_& __x) const noexcept(noexcept(__x == _v)) { 
        return __x == _v;
    }
};

template <class _Type_>
struct not_equal_to {
    _Type_ _v;

    constexpr explicit not_equal_to(_Type_ __v) noexcept: 
        _v(std::move(__v)) 
    {}

    template <class _Other_>
    raze_always_inline constexpr auto operator()(const _Other_& __x) const 
        noexcept(noexcept(__x != _v)) 
    { 
        return __x != _v;
    }
};

template <class _Type_>
struct less {
    _Type_ _v;

    constexpr explicit less(_Type_ __v) noexcept:
        _v(std::move(__v)) 
    {}

    template <class _Other_>
    raze_always_inline constexpr auto operator()(const _Other_& __x) const 
        noexcept(noexcept(__x < _v)) 
    { 
        return __x < _v;
    }
};

template <class _Type_>
using less_than = less<_Type_>;

template <class _Type_>
struct greater {
    _Type_ _v;

    constexpr explicit greater(_Type_ __v) noexcept: 
        _v(std::move(__v))
    {}

    template <class _Other_>
    raze_always_inline constexpr auto operator()(const _Other_& __x) const 
        noexcept(noexcept(__x > _v)) 
    { 
        return __x > _v;
    }
};

template <class _Type_>
using greater_than = greater<_Type_>;

template <class _Type_>
struct less_equal {
    _Type_ _v;

    constexpr explicit less_equal(_Type_ __v) noexcept: 
        _v(std::move(__v))
    {}

    template <class _Other_>
    raze_always_inline constexpr auto operator()(const _Other_& __x) const 
        noexcept(noexcept(__x <= _v)) 
    { 
        return __x <= _v;
    }
};

template <class _Type_>
struct greater_equal {
    _Type_ _v;

    constexpr explicit greater_equal(_Type_ __v) noexcept:
        _v(std::move(__v)) 
    {}

    template <class _Other_>
    raze_always_inline constexpr auto operator()(const _Other_& __x) const 
        noexcept(noexcept(__x >= _v)) 
    { 
        return __x >= _v;
    }
};


__RAZE_ALGORITHM_NAMESPACE_END
