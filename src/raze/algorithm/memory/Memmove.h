#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_never_inline void* raze_stdcall __raze_memmove(void* raze_restrict dst,
	const void* raze_restrict src, sizetype bytes) noexcept
{
	memmove(dst, src, bytes);
	return (u8*)dst + bytes;
}

__RAZE_ALGORITHM_NAMESPACE_END