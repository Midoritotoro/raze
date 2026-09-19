#pragma once

#include <src/raze/traits/IteratorCheck.h>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class T>
constexpr inline bool is_counted_iterator_helper_v = false;

template <class T>
constexpr inline bool is_counted_iterator_helper_v<std::counted_iterator<T>> = true;

template <class T>
concept counted_iterator_type = is_counted_iterator_helper_v<std::remove_cvref_t<T>>;

template <class It, class Sz>
constexpr raze_always_inline auto make_counted(It it, Sz n) {
	return std::counted_iterator<It>(std::move(it), n);
}

__RAZE_ALGORITHM_NAMESPACE_END
