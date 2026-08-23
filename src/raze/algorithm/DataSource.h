#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Range>
struct range_data_source {
	using iterator_type = decltype(std::ranges::begin(std::declval<Range>()));
	using sentinel_type = decltype(std::ranges::end(std::declval<Range>()));

	using unchecked_iterator_type = decltype(traits::ubegin(std::declval<Range>()));
	using unchecked_sentinel_type = decltype(traits::uend(std::declval<Range>()));

	constexpr explicit range_data_source(Range&& r) noexcept :
		_range(std::forward<Range>(r))
	{}

	raze_nodiscard raze_always_inline constexpr bool empty() const noexcept {
		return ubegin() == uend();
	}
	
	raze_nodiscard raze_always_inline constexpr auto size() const noexcept {
		return bytes_distance(_range);
	}

	raze_nodiscard static raze_always_inline constexpr auto static_size() noexcept
		requires(constexpr_sized_range<Range>)
	{
		return bytes_distance(options::as<std::remove_cvref_t<Range>>());
	}

	raze_nodiscard raze_always_inline constexpr iterator_type begin() const noexcept {
		return std::ranges::begin(_range);
	}

	raze_nodiscard raze_always_inline constexpr sentinel_type end() const noexcept {
		return std::ranges::end(_range);
	}

	raze_nodiscard raze_always_inline constexpr unchecked_iterator_type ubegin() const noexcept {
		return traits::ubegin(_range);
	}

	raze_nodiscard raze_always_inline constexpr unchecked_sentinel_type uend() const noexcept {
		return traits::uend(_range);
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(iterator_type it) noexcept
	{
		return traits::r_uiter<Range>(std::move(it));
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(std::iter_value_t<unchecked_iterator_type>* ptr) noexcept 
	{
		unchecked_iterator_type it;
		seek_iter(it, ptr);
		return it;
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(const std::iter_value_t<unchecked_iterator_type>* ptr) noexcept 
	{
		unchecked_iterator_type it;
		seek_iter(it, ptr);
		return it;
	}

	raze_nodiscard raze_always_inline constexpr iterator_type
		wrap(unchecked_iterator_type uit) const noexcept
	{
		iterator_type it = std::ranges::begin(_range);
		seek_iter(it, uit);
		return it;
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& uit,
		std::iter_value_t<unchecked_iterator_type>* ptr) noexcept
	{
		seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& uit,
		const std::iter_value_t<unchecked_iterator_type>* ptr) noexcept
	{
		seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& uit,
		std::iter_value_t<iterator_type>* ptr) noexcept
	{
		seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& uit,
		const std::iter_value_t<iterator_type>* ptr) noexcept
	{
		seek_iter(uit, ptr);
	}

	Range _range;
};

template <class It, class Sent>
struct iter_data_source {
	using iterator_type = It;
	using sentinel_type = Sent;

	using unchecked_iterator_type = decltype(traits::uiter<Sent>(std::declval<It>()));
	using unchecked_sentinel_type = decltype(traits::usent<It>(std::declval<Sent>()));

	constexpr iter_data_source(It it, Sent sent) noexcept :
		_it(std::move(it)), _sent(std::move(sent))
	{}

	raze_nodiscard raze_always_inline constexpr bool empty() const noexcept {
		return _it == _sent;
	}

	raze_nodiscard raze_always_inline constexpr auto size() const noexcept {
		return bytes_distance(ubegin(), uend());
	}

	raze_nodiscard raze_always_inline constexpr iterator_type begin() const noexcept {
		return _it;
	}

	raze_nodiscard raze_always_inline constexpr sentinel_type end() const noexcept {
		return _sent;
	}

	raze_nodiscard raze_always_inline constexpr unchecked_iterator_type ubegin() const noexcept {
		return traits::uiter<Sent>(_it);
	}

	raze_nodiscard raze_always_inline constexpr unchecked_sentinel_type uend() const noexcept {
		return traits::usent<It>(_sent);
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(iterator_type it) noexcept 
	{
		return traits::uiter<Sent>(std::move(it));
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(std::iter_value_t<unchecked_iterator_type>* ptr) noexcept 
	{
		unchecked_iterator_type it;
		seek_iter(it, ptr);
		return it;
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(const std::iter_value_t<unchecked_iterator_type>* ptr) noexcept
	{
		unchecked_iterator_type it;
		seek_iter(it, ptr);
		return it;
	}

	raze_nodiscard raze_always_inline constexpr iterator_type
		wrap(unchecked_iterator_type uit) const noexcept
	{
		iterator_type it = _it;
		seek_iter(it, uit);
		return it;
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& uit,
		std::iter_value_t<unchecked_iterator_type>* ptr) noexcept 
	{
		seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& uit,
		const std::iter_value_t<unchecked_iterator_type>* ptr) noexcept
	{
		seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& uit,
		std::iter_value_t<iterator_type>* ptr) noexcept
	{
		seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& uit,
		const std::iter_value_t<iterator_type>* ptr) noexcept
	{
		seek_iter(uit, ptr);
	}

	iterator_type _it;
	sentinel_type _sent;
};

template <class Range>
raze_nodiscard raze_always_inline constexpr auto get_source(Range&& r) noexcept {
	return range_data_source<Range>(std::forward<Range>(r));
}

template <class It, class Sent>
raze_nodiscard raze_always_inline constexpr auto get_source(It it, Sent sent) noexcept {
	return iter_data_source<It, Sent>(std::move(it), std::move(sent));
}

template <class Source>
concept source = requires(
	Source src,
	typename Source::iterator_type it,
	typename Source::unchecked_iterator_type uit,
	std::iter_value_t<typename Source::unchecked_iterator_type>*ptr,
	const std::iter_value_t<typename Source::unchecked_iterator_type>*cptr)
{
	typename Source::iterator_type;
	typename Source::sentinel_type;

	typename Source::unchecked_iterator_type;
	typename Source::unchecked_sentinel_type;

	{ src.empty() } noexcept -> std::convertible_to<bool>;
	{ src.size() } noexcept -> std::convertible_to<sizetype>;

	{ src.begin() } noexcept -> std::same_as<typename Source::iterator_type>;
	{ src.end() } noexcept -> std::same_as<typename Source::sentinel_type>;
	{ src.ubegin() } noexcept -> std::same_as<typename Source::unchecked_iterator_type>;
	{ src.uend() } noexcept -> std::same_as<typename Source::unchecked_sentinel_type>;

	{ Source::unwrap(it) } noexcept -> std::same_as<typename Source::unchecked_iterator_type>;
	{ Source::unwrap(ptr) } noexcept -> std::same_as<typename Source::unchecked_iterator_type>;

	{ Source::unwrap(cptr) } noexcept -> std::same_as<typename Source::unchecked_iterator_type>;
	{ src.wrap(uit) } noexcept -> std::same_as<typename Source::iterator_type>;

	{ Source::from_ptr(uit, ptr) } noexcept -> std::same_as<void>;
	{ Source::from_ptr(uit, cptr) } noexcept -> std::same_as<void>;

	{ Source::from_ptr(it, ptr) } noexcept -> std::same_as<void>;
	{ Source::from_ptr(it, cptr) } noexcept -> std::same_as<void>;
};

template <class Source>
concept constexpr_sized_source = source<Source> && requires(Source src) {
	{ Source::static_size() } noexcept -> std::convertible_to<sizetype>;
};

__RAZE_ALGORITHM_NAMESPACE_END