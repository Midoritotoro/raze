#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_always_inline void* __raze_memset(void* dst, int ch, sizetype count) noexcept {
	if (count == 0) return dst;
	return memset(dst, ch, count);
}

__RAZE_ALGORITHM_NAMESPACE_END