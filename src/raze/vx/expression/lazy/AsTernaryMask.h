#pragma once 

#include <src/raze/vx/bitwise/Ternarylogic.h>
#include <src/raze/vx/bitwise/Andnot.h>


__RAZE_VX_NAMESPACE_BEGIN

struct _A_t { 
    raze_nodiscard static constexpr bool __eval(bool __a, bool, bool) noexcept { 
        return __a; 
    } 
};

struct _B_t { 
    raze_nodiscard static constexpr bool __eval(bool, bool __b, bool) noexcept {
        return __b; 
    } 
};

struct _C_t {
    raze_nodiscard static constexpr bool __eval(bool, bool, bool __c) noexcept {
        return __c;
    } 
};

constexpr _A_t A;
constexpr _B_t B;
constexpr _C_t C;

template <
    class _Left_, 
    class _Right_>
struct _And_t {
    raze_nodiscard static constexpr bool __eval(bool __a, bool __b, bool __c) noexcept {
        return _Left_::__eval(__a, __b, __c) & _Right_::__eval(__a, __b, __c);
    } 
};

template <
    class _Left_,
    class _Right_>
struct _Or_t {
    raze_nodiscard static constexpr bool __eval(bool __a, bool __b, bool __c) noexcept {
        return _Left_::eval(__a, __b, __c) | _Right_::eval(__a, __b, __c);
    }
};

template <
    class _Left_,
    class _Right_>
struct _Xor_t {
    raze_nodiscard static constexpr bool __eval(bool __a, bool __b, bool __c) noexcept {
        return _Left_::__eval(__a, __b, __c) ^ _Right_::__eval(__a, __b, __c);
    }
};

template <class _Type_>
struct _Not_t {
    raze_nodiscard static constexpr bool __eval(bool __a, bool __b, bool __c) noexcept {
        return !_Type_::__eval(__a, __b, __c);
    }
};

template <
    class _Left_,
    class _Right_>
constexpr _And_t<_Left_, _Right_> operator&(_Left_, _Right_) noexcept {
    return {}; 
}

template <
    class _Left_,
    class _Right_>
constexpr _Or_t<_Left_, _Right_> operator|(_Left_, _Right_) noexcept {
    return {}; 
}

template <
    class _Left_,
    class _Right_>
constexpr _Xor_t<_Left_, _Right_> operator^(_Left_, _Right_) noexcept {
    return {}; 
}

template <class _Type_> 
constexpr _Not_t<_Type_> operator~(_Type_) noexcept {
    return {}; 
}

template <class _Type_>
concept lazy_expression_type =
    requires(bool __a, bool __b, bool __c) {
        { _Type_::__eval(__a, __b, __c) } -> std::same_as<bool>;
    };


template <lazy_expression_type _Expression_>
raze_nodiscard consteval uint8 __as_ternary_mask() noexcept {
    auto __imm = uint8(0);

    for (auto __mask = 0; __mask < 8; ++__mask) {
        const auto __a = static_cast<bool>(__mask & 1);
        const auto __b = static_cast<bool>(__mask & 2);
        const auto __c = static_cast<bool>(__mask & 4);

        if (_Expression_::__eval(__a, __b, __c))
            __imm |= (1u << __mask);
    }
    
    return __imm;
}

__RAZE_VX_NAMESPACE_END
