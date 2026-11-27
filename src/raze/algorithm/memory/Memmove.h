#pragma once

#include <src/raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <cstring>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_always_inline void* __raze_memmove(void* dst,
	const void* src, sizetype bytes) noexcept
{
	if (bytes == 0) return dst;
	return std::memmove(dst, src, bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END