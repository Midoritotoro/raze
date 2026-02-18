#pragma once

#include <raze/compatibility/Compatibility.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <sizetype _Size_>
struct __mmask_for_size {
	static_assert(_Size_ <= 8);
	using type = void;
};

template <>
struct __mmask_for_size<1> {
	using type = __mmask8;
};

template <>
struct __mmask_for_size<2> {
	using type = __mmask16;
};

template <>
struct __mmask_for_size<4> {
	using type = __mmask32;
};

template <>
struct __mmask_for_size<8> {
	using type = __mmask64;
};

template <sizetype _Size_>
using __mmask_for_size_t = typename __mmask_for_size<_Size_>::type;


__RAZE_DATAPAR_NAMESPACE_END
