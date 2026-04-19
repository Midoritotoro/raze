#pragma once 

#include <raze/compatibility/Compatibility.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/Arithmetic.h>
#endif

#include <src/raze/vx/hw/configurable/ConfigurableBinaryOperation.h>
#include <src/raze/vx/hw/configurable/ConfigurableUnaryOperation.h>


__RAZE_VX_NAMESPACE_BEGIN


struct saturated_mode {};

inline constexpr auto saturated = raze::options::flag(saturated_mode{});

struct saturated_option :
	raze::options::exact_option<saturated>
{};

template <class _Options_> 
using _Configurable_add = __configurable_binary_operation_type<_Add, _Options_>;

template <class _Options_>
using _Configurable_sub = __configurable_binary_operation_type<_Sub, _Options_>;

template <class _Options_>
using _Configurable_mul = __configurable_binary_operation_type<_Mul, _Options_>;

template <class _Options_>
using _Configurable_div = __configurable_binary_operation_type<_Div, _Options_>;

template <class _Options_>
using _Configurable_vertical_min = __configurable_binary_operation_type<_Vertical_min, _Options_>;

template <class _Options_>
using _Configurable_vertical_max = __configurable_binary_operation_type<_Vertical_max, _Options_>;

template <class _Options_>
using _Configurable_negate = __configurable_unary_operation_type<_Negate, _Options_>;

template <class _Options_>
using _Configurable_abs = __configurable_unary_operation_type<_Abs, _Options_>;

__RAZE_VX_NAMESPACE_END
