#pragma once

#include <raze/compatibility/Compatibility.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

enum class __prefetch_level: int32 {
	NTA = _MM_HINT_NTA,
	L1 = _MM_HINT_T0,
	L2 = _MM_HINT_T1,
	L3 = _MM_HINT_T2
};

template <__prefetch_level _Level_>
struct _Prefetch {
	template <class _InputIterator_>
	raze_static_operator raze_always_inline void operator()(_InputIterator_ __first) raze_const_operator noexcept {
		_mm_prefetch(const_cast<const char*>(reinterpret_cast<const volatile char*>(
			std::to_address(__first))), static_cast<int>(_Level_));
	}
};

__RAZE_DATAPAR_NAMESPACE_END
