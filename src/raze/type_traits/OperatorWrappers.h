#pragma once 

#include <type_traits> 

#include <raze/compatibility/Compatibility.h>
#include <raze/RazeNamespace.h>

#include <vector> 

template <class _Type_>
raze_nodiscard _Type_ __fake_copy_init(_Type_) noexcept;

__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <class _Type_ = void>
struct plus {
    raze_nodiscard constexpr _Type_ operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__left + __right))
    {
        return __left + __right;
    }
};

template <class _Type_ = void>
struct minus {
    raze_nodiscard constexpr _Type_ operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__left - __right))
    {
        return __left - __right;
    }
};

template <class _Type_ = void>
struct multiplies {
    raze_nodiscard constexpr _Type_ operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__left * __right))
    {
        return __left * __right;
    }
};

template <class _Type_ = void>
struct divides {
    raze_nodiscard constexpr _Type_ operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__left / __right))
    {
        return __left / __right;
    }
};

template <class _Type_ = void>
struct modulus {
    raze_nodiscard constexpr _Type_ operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__left % __right))
    {
        return __left % __right;
    }
};

template <class _Type_ = void>
struct negate {
    raze_nodiscard constexpr _Type_ operator()(const _Type_& __left) const noexcept(noexcept(-__left)) {
        return -__left;
    }
};


template <class _Type_ = void>
struct logical_and {
    raze_nodiscard constexpr bool operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__left && __right))
    {
        return __left && __right;
    }
};

template <class _Type_ = void>
struct logical_or {
    raze_nodiscard constexpr bool operator()(
        const _Type_& __left, 
        const _Type_& __right) const noexcept(noexcept(__left || __right))
    {
        return __left || __right;
    }
};

template <class _Type_ = void>
struct logical_not {
    raze_nodiscard constexpr bool operator()(const _Type_& __left) const noexcept(noexcept(!__left)) {
        return !__left;
    }
};

template <class _Type_ = void>
struct bit_and {
    raze_nodiscard constexpr _Type_ operator()(
        const _Type_& __left, 
        const _Type_& __right) const noexcept(noexcept(__left & __right))
    {
        return __left & __right;
    }
};

template <class _Type_ = void>
struct bit_or {
    raze_nodiscard constexpr _Type_ operator()(
        const _Type_& __left, 
        const _Type_& __right) const noexcept(noexcept(__left | __right))
    {
        return __left | __right;
    }
};

template <class _Type_ = void>
struct bit_xor {
    raze_nodiscard constexpr _Type_ operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__left ^ __right))
    {
        return __left ^ __right;
    }
};

template <class _Type_ = void>
struct bit_not {
    raze_nodiscard constexpr _Type_ operator()(const _Type_& __left) const noexcept(noexcept(~__left)) {
        return ~__left;
    }
};

template <class _Type_ = void>
struct equal_to {
    raze_nodiscard constexpr bool operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__fake_copy_init<bool>(__left == __right)))
    {
        return __left == __right;
    }
};

template <class _Type_ = void>
struct not_equal_to {
    raze_nodiscard constexpr bool operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__fake_copy_init<bool>(__left != __right)))
    {
        return __left != __right;
    }
};

template <class _Type_ = void>
struct greater {
    raze_nodiscard constexpr bool operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__fake_copy_init<bool>(__left > __right)))
    {
        return __left > __right;
    }
};

template <class _Type_ = void>
struct greater_equal {
    raze_nodiscard constexpr bool operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__fake_copy_init<bool>(__left >= __right)))
    {
        return __left >= __right;
    }
};

template <class _Type_ = void>
struct less_equal {
    raze_nodiscard constexpr bool operator()(
        const _Type_& __left,
        const _Type_& __right) const noexcept(noexcept(__fake_copy_init<bool>(__left <= __right)))
    {
        return __left <= __right;
    }
};

template <>
struct plus<void> {
    template <
        class _FirstType_, 
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&&   __left, 
        _SecondType_&&  __right) const noexcept(noexcept(static_cast<_FirstType_&&>(__left) + static_cast<_SecondType_&&>(__right)))
            -> decltype(static_cast<_FirstType_&&>(__left) + static_cast<_SecondType_&&>(__right)) 
    {
        return static_cast<_FirstType_&&>(__left) + static_cast<_SecondType_&&>(__right);
    }

    using is_transparent = int;
};

template <>
struct minus<void> {
    template <
        class _FirstType_,
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&&   __left, 
        _SecondType_&&  __right) const noexcept(noexcept(static_cast<_FirstType_&&>(__left) - static_cast<_SecondType_&&>(__right)))
            -> decltype(static_cast<_FirstType_&&>(__left) - static_cast<_SecondType_&&>(__right))
    {
        return static_cast<_FirstType_&&>(__left) - static_cast<_SecondType_&&>(__right);
    }

    using is_transparent = int;
};

template <>
struct multiplies<void> {
    template <
        class _FirstType_,
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&&   __left, 
        _SecondType_&&  __right) const noexcept(noexcept(static_cast<_FirstType_&&>(__left) * static_cast<_SecondType_&&>(__right)))
            -> decltype(static_cast<_FirstType_&&>(__left) * static_cast<_SecondType_&&>(__right)) 
    {
        return static_cast<_FirstType_&&>(__left) * static_cast<_SecondType_&&>(__right);
    }

    using is_transparent = int;
};

template <>
struct equal_to<void> {
    template <
        class _FirstType_, 
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&&   __left, 
        _SecondType_&&  __right) const noexcept(noexcept(static_cast<_FirstType_&&>(__left) == static_cast<_SecondType_&&>(__right)))
            -> decltype(static_cast<_FirstType_&&>(__left) == static_cast<_SecondType_&&>(__right)) 
    {
        return static_cast<_FirstType_&&>(__left) == static_cast<_SecondType_&&>(__right);
    }

    using is_transparent = int;
};

template <>
struct not_equal_to<void> {
    template <
        class _FirstType_, 
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&&   __left,
        _SecondType_&&  __right) const noexcept(noexcept(static_cast<_FirstType_&&>(__left) != static_cast<_SecondType_&&>(__right)))
            -> decltype(static_cast<_FirstType_&&>(__left) != static_cast<_SecondType_&&>(__right))
    {
        return static_cast<_FirstType_&&>(__left) != static_cast<_SecondType_&&>(__right);
    }

    using is_transparent = int;
};

template <>
struct greater<void> {
    template <
        class _FirstType_,
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&&   __left, 
        _SecondType_&&  __right) const noexcept(noexcept(static_cast<_FirstType_&&>(__left) > static_cast<_SecondType_&&>(__right)))
            -> decltype(static_cast<_FirstType_&&>(__left) > static_cast<_SecondType_&&>(__right)) 
    {
        return static_cast<_FirstType_&&>(__left) > static_cast<_SecondType_&&>(__right);
    }

    using is_transparent = int;
};

template <>
struct greater_equal<void> {
    template <
        class _FirstType_, 
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&& __left, 
        _SecondType_&& __right) const noexcept(noexcept(static_cast<_FirstType_&&>(__left) >= static_cast<_SecondType_&&>(__right)))
            -> decltype(static_cast<_FirstType_&&>(__left) >= static_cast<_SecondType_&&>(__right))
    {
        return static_cast<_FirstType_&&>(__left) >= static_cast<_SecondType_&&>(__right);
    }

    using is_transparent = int;
};

template <>
struct less_equal<void> {
    template <
        class _FirstType_,
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&& __left, 
        _SecondType_&& __right) const noexcept(noexcept(static_cast<_FirstType_&&>(__left) <= static_cast<_SecondType_&&>(__right)))
            -> decltype(static_cast<_FirstType_&&>(__left) <= static_cast<_SecondType_&&>(__right)) 
    {
        return static_cast<_FirstType_&&>(__left) <= static_cast<_SecondType_&&>(__right);
    }

    using is_transparent = int;
};

template <>
struct divides<void> {
    template <
        class _FirstType_, 
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&& __left, 
        _SecondType_&& __right) const noexcept(noexcept(std::forward<_FirstType_>(__left) / std::forward<_SecondType_>(__right)))
            -> decltype(std::forward<_FirstType_>(__left) / std::forward<_SecondType_>(__right)) 
    {
        return std::forward<_FirstType_>(__left) / std::forward<_SecondType_>(__right);
    }

    using is_transparent = int;
};

template <>
struct modulus<void> {
    template <
        class _FirstType_,
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&& __left,
        _SecondType_&& __right) const noexcept(noexcept(std::forward<_FirstType_>(__left) % std::forward<_SecondType_>(__right)))
            -> decltype(std::forward<_FirstType_>(__left) % std::forward<_SecondType_>(__right))
    {
        return std::forward<_FirstType_>(__left) % std::forward<_SecondType_>(__right);
    }

    using is_transparent = int;
};

template <>
struct negate<void> {
    template <class _Type_>
    raze_nodiscard constexpr auto operator()(_Type_&& __left) const noexcept(noexcept(-std::forward<_Type_>(__left))) 
        -> decltype(-std::forward<_Type_>(__left))
    {
        return -std::forward<_Type_>(__left);
    }

    using is_transparent = int;
};

template <class _Type_ = void>
struct less {
    raze_nodiscard constexpr bool operator()(
        const _Type_& __left, 
        const _Type_& __right) const noexcept(noexcept(__fake_copy_init<bool>(__left < __right)))
    {
        return __left < __right;
    }
};

template <>
struct less<void> {
    template <
        class _FirstType_, 
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&&   __left, 
        _SecondType_&&  __right) const noexcept(noexcept(static_cast<_FirstType_&&>(__left) < static_cast<_SecondType_&&>(__right)))
            -> decltype(static_cast<_FirstType_&&>(__left) < static_cast<_SecondType_&&>(__right))
    {
        return static_cast<_FirstType_&&>(__left) < static_cast<_SecondType_&&>(__right);
    }

    using is_transparent = int;
};

template <>
struct logical_and<void> {
    template <
        class _FirstType_, 
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&& __left,
        _SecondType_&& __right) const noexcept(noexcept(std::forward<_FirstType_>(__left) && std::forward<_SecondType_>(__right)))
            -> decltype(std::forward<_FirstType_>(__left) && std::forward<_SecondType_>(__right)) {
        return std::forward<_FirstType_>(__left) && std::forward<_SecondType_>(__right);
    }

    using is_transparent = int;
};

template <>
struct logical_or<void> {
    template <
        class _FirstType_, 
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&& __left, 
        _SecondType_&& __right) const noexcept(noexcept(std::forward<_FirstType_>(__left) || std::forward<_SecondType_>(__right)))
            -> decltype(std::forward<_FirstType_>(__left) || std::forward<_SecondType_>(__right))
    {
        return std::forward<_FirstType_>(__left) || std::forward<_SecondType_>(__right);
    }

    using is_transparent = int;
};

template <>
struct logical_not<void> {
    template <class _Type_>
    raze_nodiscard constexpr auto operator()(_Type_&& __left) const noexcept(noexcept(!std::forward<_Type_>(__left))) 
        -> decltype(!std::forward<_Type_>(__left))
    {
        return !std::forward<_Type_>(__left);
    }

    using is_transparent = int;
};

template <>
struct bit_and<void> {
    template <
        class _FirstType_,
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&&   __left, 
        _SecondType_&&  __right) const noexcept(noexcept(std::forward<_FirstType_>(__left) & std::forward<_SecondType_>(__right)))
            -> decltype(std::forward<_FirstType_>(__left)& std::forward<_SecondType_>(__right)) 
    {
        return std::forward<_FirstType_>(__left) & std::forward<_SecondType_>(__right);
    }

    using is_transparent = int;
};

template <>
struct bit_or<void> {
    template <
        class _FirstType_, 
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&& __left,
        _SecondType_&& __right) const noexcept(noexcept(std::forward<_FirstType_>(__left) | std::forward<_SecondType_>(__right)))
            -> decltype(std::forward<_FirstType_>(__left) | std::forward<_SecondType_>(__right)) 
    {
        return std::forward<_FirstType_>(__left) | std::forward<_SecondType_>(__right);
    }

    using is_transparent = int;
};

template <>
struct bit_xor<void> {
    template <
        class _FirstType_,
        class _SecondType_>
    raze_nodiscard constexpr auto operator()(
        _FirstType_&&   __left,
        _SecondType_&&  __right) const noexcept(noexcept(std::forward<_FirstType_>(__left) ^ std::forward<_SecondType_>(__right)))
            -> decltype(std::forward<_FirstType_>(__left) ^ std::forward<_SecondType_>(__right))
    {
        return std::forward<_FirstType_>(__left) ^ std::forward<_SecondType_>(__right);
    }

    using is_transparent = int;
};

template <>
struct bit_not<void> {
    template <class _Type_>
    raze_nodiscard constexpr auto operator()(_Type_&& __left) const noexcept(noexcept(~std::forward<_Type_>(__left))) 
        -> decltype(~std::forward<_Type_>(__left))
    {
        return ~std::forward<_Type_>(__left);
    }

    using is_transparent = int;
};


__RAZE_TYPE_TRAITS_NAMESPACE_END
