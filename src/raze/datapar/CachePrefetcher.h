#pragma once

#include <raze/compatibility/Compatibility.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

enum class __prefetch_hint: int32 {
	NTA = _MM_HINT_NTA,
	T0  = _MM_HINT_T0,
	T1  = _MM_HINT_T1,
	T2  = _MM_HINT_T2
};

template <__prefetch_hint _Hint_>
class __cache_prefetcher {
public:
	static constexpr auto __prefetch_stride = 0x40;

	template <class _Pointer_>
	raze_always_inline void operator()(_Pointer_ __pointer) const noexcept {
		_mm_prefetch(const_cast<const char*>(reinterpret_cast<const volatile char*>(std::to_address(__pointer))), static_cast<int>(_Hint_));
	}
};

__RAZE_DATAPAR_NAMESPACE_END