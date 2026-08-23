#pragma once 

#include <raze/compatibility/CompilerDetection.h>
#include <raze/RazeNamespace.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <src/raze/algorithm/AlgorithmDebug.h>
#include <xutility>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class It>
raze_always_inline constexpr decltype(auto) uiter(It&& it)
    noexcept(traits::is_iterator_unwrappable_v<It> == false || traits::is_nothrow_unwrappable_v<It>)
{
    if constexpr (std::is_pointer_v<std::decay_t<It>>) return it + 0;
    else if constexpr (traits::is_iterator_unwrappable_v<It>) return std::move(it)._Unwrapped();
    else return std::move(it);
}

template <class It>
using unwrapped_iterator_type = std::remove_cvref_t<decltype(uiter(std::declval<It>()))>;

template <class It, class UIt>
raze_always_inline constexpr void seek_iter(It& it, UIt&& uit) noexcept(
        traits::is_wrapped_iterator_seekable_v<It, UIt> == false || 
        traits::is_wrapped_iterator_nothrow_seekable_v<It, UIt>)
{
    if constexpr (traits::is_wrapped_iterator_seekable_v<It, UIt>)
        it._Seek_to(std::forward<UIt>(uit));
    else it = std::forward<UIt>(uit);
}
__RAZE_ALGORITHM_NAMESPACE_END
