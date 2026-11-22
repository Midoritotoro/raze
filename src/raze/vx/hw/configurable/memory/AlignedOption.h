#pragma once 

#include <raze/options/Options.h>


__RAZE_VX_NAMESPACE_BEGIN

struct aligned_mode {};

/**
 * @ingroup raze_vx_memory
 * @memberof raze::vx
 * @hideinitializer
 * @brief Memory access option instructing `load` / `store` / `compress_store` to use aligned SIMD instructions.
 *
 * The target pointer must be aligned to at least `alignof(V)`.
*/
constexpr inline auto aligned = raze::options::flag(aligned_mode{});
struct aligned_option : raze::options::exact_option<aligned> {};
 
struct safe_mode {};

/**
 * @ingroup raze_vx_memory
 * @memberof raze::vx
 * @hideinitializer
 * @brief Memory access option guaranteeing that masked-out memory lanes are not touched.
 *
 * Prevents speculative or full-vector memory reads/writes on unselected lanes, avoiding
 * page faults near unmapped memory boundaries on architectures without hardware masked instructions.
*/
constexpr inline auto safe = raze::options::flag(safe_mode{});
struct safe_option : raze::options::exact_option<safe> {};

__RAZE_VX_NAMESPACE_END
