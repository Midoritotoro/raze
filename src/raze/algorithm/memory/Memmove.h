#pragma once 

#include <raze/compatibility/Compatibility.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_declare_const_function void* raze_cdecl raze_memmove(
	void* dst, const void* src, sizetype bytes) noexcept;

__RAZE_ALGORITHM_NAMESPACE_END
