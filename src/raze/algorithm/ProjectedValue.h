#pragma once

#include <raze/RazeNamespace.h>
#include <iterator>
#include <type_traits>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

/**
 * @ingroup raze_algo_concepts
 * @brief C++26 P2248 `projected_value_t` alias implemented for C++23.
 *
 * Computes the decayed result of invoking projection `Proj` on an lvalue of `std::iter_value_t<It>`,
 * enabling braced-init-list value arguments (e.g., `find(v, {1, 2})`) on algorithms taking a value parameter.
 */
template <std::indirectly_readable It, std::indirectly_regular_unary_invocable<It> Proj>
using projected_value_t = std::remove_cvref_t<std::invoke_result_t<Proj&, std::iter_value_t<It>&>>;

__RAZE_ALGORITHM_NAMESPACE_END

