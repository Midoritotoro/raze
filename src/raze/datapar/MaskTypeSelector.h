#pragma once

#include <raze/compatibility/Compatibility.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <sizetype _Elements_>
using __mmask_for_elements_helper = std::conditional_t<_Elements_ <= 8, __mmask8,
	std::conditional_t<_Elements_ <= 16, __mmask16,
		std::conditional_t<_Elements_ <= 32, __mmask32,
			std::conditional_t<_Elements_ <= 64, __mmask64, void>>>>;

template <sizetype _Elements_>
using __mmask_for_elements_t = __mmask_for_elements_helper<_Elements_>;

__RAZE_DATAPAR_NAMESPACE_END
