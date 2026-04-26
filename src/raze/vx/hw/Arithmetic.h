#pragma once 

#include <src/raze/vx/hw/configurable/arithmetic/Add.h>
#include <src/raze/vx/hw/configurable/arithmetic/Sub.h>
#include <src/raze/vx/hw/configurable/arithmetic/Mul.h>
#include <src/raze/vx/hw/configurable/arithmetic/Div.h>
#include <src/raze/vx/hw/configurable/arithmetic/Negate.h>
#include <src/raze/vx/hw/configurable/arithmetic/Abs.h>
#include <src/raze/vx/hw/configurable/arithmetic/VerticalMax.h>
#include <src/raze/vx/hw/configurable/arithmetic/VerticalMin.h>


#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/Arithmetic.h>
#endif

__RAZE_VX_NAMESPACE_BEGIN


struct saturated_mode {};

inline constexpr auto saturated = raze::options::flag(saturated_mode{});

struct saturated_option :
	raze::options::exact_option<saturated>
{};

__RAZE_VX_NAMESPACE_END
