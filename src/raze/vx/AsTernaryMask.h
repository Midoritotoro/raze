#pragma once 

#include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>
#include <src/raze/vx/hw/x86/bitwise/Andnot.h>


__RAZE_VX_NAMESPACE_BEGIN

struct A_t { 
    raze_nodiscard static constexpr bool eval(bool a, bool, bool) noexcept { 
        return a; 
    } 
};

struct B_t { 
    raze_nodiscard static constexpr bool eval(bool, bool b, bool) noexcept {
        return b; 
    } 
};

struct C_t {
    raze_nodiscard static constexpr bool eval(bool, bool, bool c) noexcept {
        return c;
    } 
};

constexpr A_t A;
constexpr B_t B;
constexpr C_t C;

template <class L, class R>
struct And_t {
    raze_nodiscard static constexpr bool eval(bool a, bool b, bool c) noexcept {
        return L::eval(a, b, c) & R::eval(a, b, c);
    } 
};

template <class L, class R>
struct Or_t {
    raze_nodiscard static constexpr bool eval(bool a, bool b, bool c) noexcept {
        return L::eval(a, b, c) | R::eval(a, b, c);
    }
};

template <class L, class R>
struct Xor_t {
    raze_nodiscard static constexpr bool eval(bool a, bool b, bool c) noexcept {
        return L::eval(a, b, c) ^ R::eval(a, b, c);
    }
};

template <class T>
struct Not_t {
    raze_nodiscard static constexpr bool eval(bool a, bool b, bool c) noexcept {
        return !T::eval(a, b, c);
    }
};

template <class T>
concept ternary_mask_expression_type =
    requires(bool a, bool b, bool c) {
        { T::eval(a, b, c) } -> std::same_as<bool>;
    };


template <ternary_mask_expression_type L, ternary_mask_expression_type R>
constexpr And_t<L, R> operator&(L, R) noexcept {
    return {}; 
}

template <ternary_mask_expression_type L, ternary_mask_expression_type R>
constexpr Or_t<L, R> operator|(L, R) noexcept {
    return {}; 
}

template <ternary_mask_expression_type L, ternary_mask_expression_type R>
constexpr Xor_t<L, R> operator^(L, R) noexcept {
    return {}; 
}

template <ternary_mask_expression_type T>
constexpr Not_t<T> operator~(T) noexcept {
    return {}; 
}

template <ternary_mask_expression_type Expr>
raze_nodiscard consteval u8 as_ternary_mask_impl() noexcept {
    auto imm = u8(0);

    for (auto mask = 0; mask < 8; ++mask) {
        const auto a = static_cast<bool>(mask & 1);
        const auto b = static_cast<bool>(mask & 2);
        const auto c = static_cast<bool>(mask & 4);

        if (Expr::eval(a, b, c))
            imm |= (1u << mask);
    }
    
    return imm;
}

__RAZE_VX_NAMESPACE_END
