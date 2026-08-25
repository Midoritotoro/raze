#pragma once

#include <raze/compatibility/Compatibility.h>


__RAZE_VX_NAMESPACE_BEGIN

enum class prefetch_level: i32 {
	NTA = _MM_HINT_NTA,
	L1 = _MM_HINT_T0,
	L2 = _MM_HINT_T1,
	L3 = _MM_HINT_T2
};

template <prefetch_level Level, any_iterator_or_pointer Mem>
raze_always_inline void prefetch_(Mem ptr) noexcept {
	_mm_prefetch(const_cast<const char*>(reinterpret_cast<const volatile char*>(
		std::to_address(ptr))), static_cast<int>(Level));
}

__RAZE_VX_NAMESPACE_END
