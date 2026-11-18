#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_never_inline void* __raze_memset(void* dst, int ch, sizetype count) noexcept {
	return memset(dst, ch, count);
}

__RAZE_ALGORITHM_NAMESPACE_END