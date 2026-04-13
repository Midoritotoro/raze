#pragma once 

#include <raze/compatibility/Compatibility.h>


__RAZE_OPTIONS_CONCEPTS_NAMESPACE_BEGIN

template <
    class _Type1_,
    class _Type2_>
concept __is_same = std::is_same_v<_Type1_, _Type2_>;

template <
    class _Type1_,
    class _Type2_>
concept same_as = __is_same<_Type1_, _Type2_> && __is_same<_Type2_, _Type1_>;

__RAZE_OPTIONS_CONCEPTS_NAMESPACE_END

