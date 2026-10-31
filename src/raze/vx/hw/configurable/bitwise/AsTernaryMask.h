#pragma once

#include <src/raze/vx/AsTernaryMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <ternary_mask_expression_type Expression>
struct as_ternary_mask_t {
	raze_always_inline constexpr auto operator()() noexcept {
		return as_ternary_mask_impl<Expression>();
	}
};

/**
 * @ingroup raze_vx_bitwise
 * @memberof raze::vx
 * @hideinitializer
 * @brief Generates an 8-bit truth table immediate for `ternarylogic` from a compile-time logical expression.
 *
 * Allows constructing the `Op` immediate required by `ternarylogic` using standard logical operators
 * (`&`, `|`, `^`, `~`) over the symbolic variables `raze::vx::A`, `raze::vx::B`, and `raze::vx::C`.
 * The expression is evaluated for all 8 input combinations at compile time to compute the truth table.
 *
 * ### Declarations
 * ```cpp
 * template <ternary_mask_expression_type Expression>
 * constexpr inline *unspecified* as_ternary_mask;
 * ```
 *
 * ### Template Parameters
 * - `Expression` — A compile-time AST representing the logical function (e.g., `decltype(A & B)`).
 *
 * ### Return value
 * An 8-bit immediate (`u8`) representing the truth table of the expression, suitable for `ternarylogic`.
 *
 * ### Usage
 * ```cpp
 * // Instead of manually calculating the 8-bit magic constant:
 * auto r = raze::vx::ternarylogic<T>(x, y, z, raze::vx::as_ternary_mask<decltype(raze::vx::A & ~raze::vx::B | raze::vx::C)>());
 * ```
*/
template <ternary_mask_expression_type Expression>
constexpr inline as_ternary_mask_t<Expression> as_ternary_mask;

__RAZE_VX_NAMESPACE_END