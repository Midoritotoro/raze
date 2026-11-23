#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

void* __raze_memmove(void* raze_restrict dst,
	const void* raze_restrict src, sizetype bytes) noexcept
{
	return memmove(dst, src, bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END