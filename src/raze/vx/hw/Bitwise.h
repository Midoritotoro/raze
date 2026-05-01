#pragma once 

#include <src/raze/vx/hw/configurable/bitwise/And.h>
#include <src/raze/vx/hw/configurable/bitwise/Andnot.h>
#include <src/raze/vx/hw/configurable/bitwise/Or.h>
#include <src/raze/vx/hw/configurable/bitwise/Xor.h>
#include <src/raze/vx/hw/configurable/bitwise/Not.h>
#include <src/raze/vx/hw/configurable/bitwise/ShiftLeft.h>
#include <src/raze/vx/hw/configurable/bitwise/ShiftRight.h>


#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/Bitwise.h>
#endif

__RAZE_VX_NAMESPACE_BEGIN


__RAZE_VX_NAMESPACE_END
