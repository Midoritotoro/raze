#pragma once 

#include <raze/compatibility/Compatibility.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_declare_const_function void* raze_cdecl __raze_memmove(
	void* __dst, const void* __src, sizetype __bytes) noexcept;

__RAZE_ALGORITHM_NAMESPACE_END
