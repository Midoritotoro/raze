#pragma once

#include <raze/compatibility/Compatibility.h>


__RAZE_VX_NAMESPACE_BEGIN

raze_always_inline void __x86_store_fence() noexcept {
	_mm_sfence();
}

__RAZE_VX_NAMESPACE_END
