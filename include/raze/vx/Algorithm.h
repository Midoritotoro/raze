#pragma once 

#include <src/raze/vx/hw/Reduce.h>
#include <src/raze/vx/hw/Compact.h>
#include <src/raze/vx/hw/Compare.h>
#include <src/raze/vx/hw/Construct.h>
#include <src/raze/vx/hw/Shuffle.h>
#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/vx/hw/Bitwise.h>
#include <src/raze/vx/hw/Mask.h>
#include <src/raze/vx/hw/Merge.h>
#include <src/raze/vx/AsTernaryMask.h>


__RAZE_VX_NAMESPACE_BEGIN

constexpr inline auto add = raze::options::functor<_Configurable_add>;
constexpr inline auto sub = raze::options::functor<_Configurable_sub>;
constexpr inline auto mul = raze::options::functor<_Configurable_mul>;
constexpr inline auto div = raze::options::functor<_Configurable_div>;
constexpr inline auto vertical_max = raze::options::functor<_Configurable_vmax>;
constexpr inline auto vertical_min = raze::options::functor<_Configurable_vmin>;
constexpr inline auto neg = raze::options::functor<_Configurable_neg>;
constexpr inline auto abs = raze::options::functor<_Configurable_abs>;
constexpr inline auto bit_or = raze::options::functor<_Configurable_or>;
constexpr inline auto bit_xor = raze::options::functor<_Configurable_xor>;
constexpr inline auto bit_and = raze::options::functor<_Configurable_and>;
constexpr inline auto bit_andnot = raze::options::functor<_Configurable_andnot>;
constexpr inline auto bit_shl = raze::options::functor<_Configurable_shl>;
constexpr inline auto bit_shr = raze::options::functor<_Configurable_shr>;
constexpr inline auto bit_not = raze::options::functor<_Configurable_not>;
constexpr inline auto horizontal_sum = raze::options::functor<_Configurable_hsum>;
constexpr inline auto horizontal_min = raze::options::functor<_Configurable_hmin>;
constexpr inline auto horizontal_max = raze::options::functor<_Configurable_hmax>;
constexpr inline auto all_of = raze::options::functor<_Configurable_all_of>;
constexpr inline auto none_of = raze::options::functor<_Configurable_none_of>;
constexpr inline auto any_of = raze::options::functor<_Configurable_any_of>;
constexpr inline auto some_of = raze::options::functor<_Configurable_some_of>;
constexpr inline auto is_contiguous = raze::options::functor<_Configurable_is_contiguous>;
constexpr inline auto is_less = raze::options::functor<_Configurable_is_less>;
constexpr inline auto is_greater = raze::options::functor<_Configurable_is_greater>;
constexpr inline auto is_less_equal = raze::options::functor<_Configurable_is_less_equal>;
constexpr inline auto is_greater_equal = raze::options::functor<_Configurable_is_greater_equal>;
constexpr inline auto is_equal = raze::options::functor<_Configurable_is_equal>;
constexpr inline auto is_not_equal = raze::options::functor<_Configurable_is_not_equal>;
constexpr inline auto select = raze::options::functor<_Configurable_select>;
constexpr inline auto ternarylogic = raze::options::functor<_Configurable_ternarylogic>;
constexpr inline auto vmin = vertical_min;
constexpr inline auto vmax = vertical_max;
constexpr inline auto hsum = horizontal_sum;
constexpr inline auto hmin = horizontal_min;
constexpr inline auto hmax = horizontal_max;

template <ternary_mask_expression_type _Expression_>
raze_always_inline constexpr auto as_ternary_mask() noexcept {
	return __as_ternary_mask<_Expression_>();
}


__RAZE_VX_NAMESPACE_END
