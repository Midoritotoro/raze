#pragma once 

#include <raze/options/Options.h>


__RAZE_VX_NAMESPACE_BEGIN

struct not_null_mode {};

/**
 * @ingroup raze_vx_mask_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Optimization hint asserting that the input mask contains at least one active/target lane.
 *
 * Used with `find_first_set`, `find_first_not_set`, `find_last_set`, `find_last_not_set`,
 * `find_next_set`, `clear_first`, and `clear_last` to omit zero-mask checks.
 * Invoking an operation with `not_null` on a mask that violates this precondition results in undefined behavior.
*/
constexpr inline auto not_null = options::flag(not_null_mode{});
struct not_null_option : options::exact_option<not_null> {};

__RAZE_VX_NAMESPACE_END
