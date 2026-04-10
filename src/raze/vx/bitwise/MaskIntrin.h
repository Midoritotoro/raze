#pragma once 

#include <raze/compatibility/Compatibility.h>


__RAZE_VX_NAMESPACE_BEGIN

__raze_msvc_intrin_workaround bool __ktestcb(__mmask8 __x, __mmask8 __y) noexcept {
	return _ktestc_mask8_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __ktestcw(__mmask16 __x, __mmask16 __y) noexcept {
	return _ktestc_mask16_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __ktestcd(__mmask32 __x, __mmask32 __y) noexcept {
	return _ktestc_mask32_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __ktestcq(__mmask64 __x, __mmask64 __y) noexcept {
	return _ktestc_mask64_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __kortestcb(__mmask8 __x, __mmask8 __y) noexcept {
	return _kortestc_mask8_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __kortestcw(__mmask16 __x, __mmask16 __y) noexcept {
	return _kortestc_mask16_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __kortestcd(__mmask32 __x, __mmask32 __y) noexcept {
	return _kortestc_mask32_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __kortestcq(__mmask64 __x, __mmask64 __y) noexcept {
	return _kortestc_mask64_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __kortestzb(__mmask8 __x, __mmask8 __y) noexcept {
	return _kortestz_mask8_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __kortestzw(__mmask16 __x, __mmask16 __y) noexcept {
	return _kortestz_mask16_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __kortestzd(__mmask32 __x, __mmask32 __y) noexcept {
	return _kortestz_mask32_u8(__x, __y);
}

__raze_msvc_intrin_workaround bool __kortestzq(__mmask64 __x, __mmask64 __y) noexcept {
	return _kortestz_mask64_u8(__x, __y);
}

__RAZE_VX_NAMESPACE_END
