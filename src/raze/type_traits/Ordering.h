#pragma once 

#include <type_traits>
#include <raze/compatibility/CxxVersionDetection.h>


#if raze_has_cxx20

#include <compare>

#include <raze/RazeNamespace.h>
#include <raze/compatibility/Nodiscard.h>

__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <class _Type_>
concept __boolean_testable = std::convertible_to<_Type_, bool>;

template <class _Type_>
concept boolean_testable = __boolean_testable<_Type_> && requires(_Type_ && __t) {
    { !static_cast<_Type_&&>(__t) } -> __boolean_testable;
};

template <
    class _FirstType_,
    class _SecondType_>
concept can_strong_order = requires(
    _FirstType_ & __left,
    _SecondType_& __right)
{
    std::strong_order(__left, __right);
};

template <
    class _FirstType_,
    class _SecondType_>
concept can_weak_order = requires(
    _FirstType_&    __left,
    _SecondType_&   __right)
{
    std::weak_order(__left, __right);
};

template <
    class _FirstType_,
    class _SecondType_>
concept can_partial_order = requires(
    _FirstType_&    __left,
    _SecondType_&   __right)
{
    std::partial_order(__left, __right);
};


struct synthetic_three_way {
    template <
        class _FirstType_,
        class _SecondType_>
    raze_nodiscard static constexpr auto operator()(
        const _FirstType_& __left,
        const _SecondType_& __right)
            requires requires {
                { __left < __right } -> boolean_testable;
                { __right < __left } -> boolean_testable;
        }
    {
        if constexpr (std::three_way_comparable_with<_FirstType_, _SecondType_>) {
            return __left <=> __right;
        }
        else {
            if (__left < __right)
                return std::weak_ordering::less;

            else if (__right < __left)
                return std::weak_ordering::greater;

            else
                return std::weak_ordering::equivalent;
        }
    }
};

__RAZE_TYPE_TRAITS_NAMESPACE_END

#endif // base_has_cxx20
