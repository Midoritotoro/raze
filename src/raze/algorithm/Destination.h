#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <src/raze/traits/IteratorCheck.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class It, bool = std::contiguous_iterator<traits::unwrapped_iterator_type<It>>>
struct data_destination;

template <class It>
struct data_destination<It, false> {
	using iterator_type = It;
	using unchecked_iterator_type = traits::unwrapped_iterator_type<It>;

	constexpr explicit data_destination(It it):
		_it(std::move(it))
	{}

	raze_nodiscard raze_always_inline constexpr iterator_type begin() const {
		return _it;
	}

	raze_nodiscard raze_always_inline constexpr unchecked_iterator_type ubegin() const {
		return traits::uiter(_it);
	}

	template <class I>
	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(I it) requires(std::same_as<I, iterator_type>)
	{
		return traits::uiter(std::move(it));
	}

	raze_nodiscard raze_always_inline constexpr iterator_type
		wrap(unchecked_iterator_type uit) const
	{
		if constexpr (std::constructible_from<iterator_type, unchecked_iterator_type>) return iterator_type(std::move(uit));
		else {
			iterator_type it = _it;
			traits::seek_iter(it, uit);
			return it;
		}
	}

	iterator_type _it;
};

template <class It>
struct data_destination<It, true> {
	using iterator_type = It;
	using unchecked_iterator_type = traits::unwrapped_iterator_type<It>;
	using value_type = std::iter_value_t<unchecked_iterator_type>;

	constexpr explicit data_destination(It it):
		_it(std::move(it))
	{}

	raze_nodiscard raze_always_inline constexpr iterator_type begin() const {
		return _it;
	}

	raze_nodiscard raze_always_inline constexpr unchecked_iterator_type ubegin() const {
		return traits::uiter(_it);
	}

	template <class I>
	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(I it) requires(std::same_as<I, iterator_type>)
	{
		return traits::uiter(std::move(it));
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type unwrap(value_type* ptr) {
		unchecked_iterator_type it;
		traits::seek_iter(it, ptr);
		return it;
	}

	raze_nodiscard raze_always_inline constexpr iterator_type
		wrap(unchecked_iterator_type uit) const
	{
		iterator_type it = _it;
		traits::seek_iter(it, uit);
		return it;
	}

	template <class I>
	static raze_always_inline constexpr void from_ptr(I& it, value_type* ptr)
		requires(std::same_as<I, iterator_type> || std::same_as<I, unchecked_iterator_type>)
	{
		traits::seek_iter(it, ptr);
	}

	iterator_type _it;
};

template <class It>
raze_nodiscard raze_always_inline constexpr auto get_destination(It it) {
	return data_destination<It>(std::move(it));
}

template <class Destination>
concept destination = requires(
	Destination dest,
	typename Destination::iterator_type it)
{
	typename Destination::iterator_type;
	typename Destination::unchecked_iterator_type;

	{ dest.begin() } -> std::same_as<typename Destination::iterator_type>;
	{ dest.ubegin() } -> std::same_as<typename Destination::unchecked_iterator_type>;

	{ Destination::unwrap(it) } -> std::same_as<
		typename Destination::unchecked_iterator_type>;
};

template <class Destination>
concept contiguous_destination =
	destination<Destination> &&
	std::contiguous_iterator<typename Destination::iterator_type> &&
	std::contiguous_iterator<typename Destination::unchecked_iterator_type> &&
	requires(
		Destination dest,
		typename Destination::iterator_type it,
		typename Destination::unchecked_iterator_type uit,
		std::iter_value_t<typename Destination::unchecked_iterator_type>* ptr)
{
		{ Destination::unwrap(ptr) } -> std::same_as<
			typename Destination::unchecked_iterator_type>;

		{ dest.wrap(uit) } -> std::same_as<
			typename Destination::iterator_type>;

		{ Destination::from_ptr(uit, ptr) } -> std::same_as<void>;
	};

__RAZE_ALGORITHM_NAMESPACE_END