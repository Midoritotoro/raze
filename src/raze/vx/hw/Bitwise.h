#pragma once 

#include <raze/compatibility/Compatibility.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/Bitwise.h>
#endif

#include <src/raze/vx/hw/configurable/ConfigurableBinaryOperation.h>
#include <src/raze/vx/hw/configurable/ConfigurableUnaryOperation.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_> 
using _Configurable_or = __configurable_binary_operation_type<_Or, _Options_>;

template <class _Options_>
using _Configurable_xor = __configurable_binary_operation_type<_Xor, _Options_>;

template <class _Options_>
using _Configurable_and = __configurable_binary_operation_type<_And, _Options_>;

template <class _Options_>
using _Configurable_andnot = __configurable_binary_operation_type<_Andnot, _Options_>;
//
//template <class _Options_>
//using _Configurable_shl = __configurable_binary_operation_type<_Left_shift, _Options_>;
//
//template <class _Options_>
//using _Configurable_shr = __configurable_binary_operation_type<_Right_shift, _Options_>;

template <class _Options_>
using _Configurable_not = __configurable_unary_operation_type<_Not, _Options_>;

__RAZE_VX_NAMESPACE_END
