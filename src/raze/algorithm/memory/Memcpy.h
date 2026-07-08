#pragma once 

#include <raze/compatibility/Compatibility.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_declare_const_function void* raze_cdecl __raze_memcpy(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __bytes) noexcept;

__RAZE_ALGORITHM_NAMESPACE_END
