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

	constexpr explicit range_data_source(Range&& r) :
		_range(std::forward<Range>(r))
	{}

	raze_nodiscard raze_always_inline constexpr bool empty() const {
		return ubegin() == uend();
	}
	
	raze_nodiscard raze_always_inline constexpr auto size() const {
		return bytes_distance(_range);
	}

	raze_nodiscard static raze_always_inline constexpr auto static_size()
		requires(constexpr_sized_range<Range>)
	{
		return bytes_distance(options::as<std::remove_cvref_t<Range>>());
	}

	raze_nodiscard raze_always_inline constexpr iterator_type begin() const {
		return std::ranges::begin(_range);
	}

	raze_nodiscard raze_always_inline constexpr sentinel_type end() const {
		return std::ranges::end(_range);
	}

	raze_nodiscard raze_always_inline constexpr unchecked_iterator_type ubegin() const {
		return traits::ubegin(_range);
	}

	raze_nodiscard raze_always_inline constexpr unchecked_sentinel_type uend() const {
		return traits::uend(_range);
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(iterator_type it)
	{
		return traits::r_uiter<Range>(std::move(it));
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(std::iter_value_t<unchecked_iterator_type>* ptr) 
	{
		unchecked_iterator_type it;
		traits::seek_iter(it, ptr);
		return it;
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(const std::iter_value_t<unchecked_iterator_type>* ptr) 
	{
		unchecked_iterator_type it;
		traits::seek_iter(it, ptr);
		return it;
	}

	raze_nodiscard raze_always_inline constexpr iterator_type
		wrap(unchecked_iterator_type uit) const
	{
		iterator_type it = std::ranges::begin(_range);
		traits::seek_iter(it, uit);
		return it;
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& uit,
		std::iter_value_t<unchecked_iterator_type>* ptr)
	{
		traits::seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& uit,
		const std::iter_value_t<unchecked_iterator_type>* ptr)
	{
		traits::seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& uit,
		std::iter_value_t<iterator_type>* ptr)
	{
		traits::seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& uit,
		const std::iter_value_t<iterator_type>* ptr)
	{
		traits::seek_iter(uit, ptr);
	}

	Range _range;
};

template <class It, class Sent>
struct iter_data_source {
	using iterator_type = It;
	using sentinel_type = Sent;

	using unchecked_iterator_type = decltype(traits::uiter_s<Sent>(std::declval<It>()));
	using unchecked_sentinel_type = decltype(traits::usent<It>(std::declval<Sent>()));

	constexpr iter_data_source(It it, Sent sent) :
		_it(std::move(it)), _sent(std::move(sent))
	{}

	raze_nodiscard raze_always_inline constexpr bool empty() const {
		return _it == _sent;
	}

	raze_nodiscard raze_always_inline constexpr auto size() const {
		return bytes_distance(ubegin(), uend());
	}

	raze_nodiscard raze_always_inline constexpr iterator_type begin() const {
		return _it;
	}

	raze_nodiscard raze_always_inline constexpr sentinel_type end() const {
		return _sent;
	}

	raze_nodiscard raze_always_inline constexpr unchecked_iterator_type ubegin() const {
		return traits::uiter_s<Sent>(_it);
	}

	raze_nodiscard raze_always_inline constexpr unchecked_sentinel_type uend() const {
		return traits::usent<It>(_sent);
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(iterator_type it) 
	{
		return traits::uiter_s<Sent>(std::move(it));
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(std::iter_value_t<unchecked_iterator_type>* ptr) 
	{
		unchecked_iterator_type it;
		traits::seek_iter(it, ptr);
		return it;
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(const std::iter_value_t<unchecked_iterator_type>* ptr)
	{
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

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& uit,
		std::iter_value_t<unchecked_iterator_type>* ptr)
			requires(!std::same_as< unchecked_iterator_type, iterator_type>)
	{
		traits::seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& uit,
		const std::iter_value_t<unchecked_iterator_type>* ptr) 
			requires(!std::same_as< unchecked_iterator_type, iterator_type>)
	{
		traits::seek_iter(uit, ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& it, std::iter_value_t<iterator_type>* ptr) {
		traits::seek_iter(it, ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& it, const std::iter_value_t<iterator_type>* ptr) {
		traits::seek_iter(it, ptr);
	}

	iterator_type _it;
	sentinel_type _sent;
};

template <class Range>
raze_nodiscard raze_always_inline constexpr auto get_source(Range&& r) {
	return range_data_source<Range>(std::forward<Range>(r));
}

template <class It, class Sent>
raze_nodiscard raze_always_inline constexpr auto get_source(It it, Sent sent) {
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

	{ src.empty() } -> std::convertible_to<bool>;
	{ src.size() } -> std::convertible_to<sizetype>;

	{ src.begin() } -> std::same_as<typename Source::iterator_type>;
	{ src.end() } -> std::same_as<typename Source::sentinel_type>;
	{ src.ubegin() } -> std::same_as<typename Source::unchecked_iterator_type>;
	{ src.uend() } -> std::same_as<typename Source::unchecked_sentinel_type>;

	{ Source::unwrap(it) } -> std::same_as<typename Source::unchecked_iterator_type>;
	{ Source::unwrap(ptr) } -> std::same_as<typename Source::unchecked_iterator_type>;

	{ Source::unwrap(cptr) } -> std::same_as<typename Source::unchecked_iterator_type>;
	{ src.wrap(uit) } -> std::same_as<typename Source::iterator_type>;

	{ Source::from_ptr(uit, ptr) } -> std::same_as<void>;
	{ Source::from_ptr(uit, cptr) } -> std::same_as<void>;

	{ Source::from_ptr(it, ptr) } -> std::same_as<void>;
	{ Source::from_ptr(it, cptr) } -> std::same_as<void>;
};

template <class Source>
concept constexpr_sized_source = source<Source> && requires(Source src) {
	{ Source::static_size() } -> std::convertible_to<sizetype>;
};

template <class Source>
concept contiguous_source = source<Source> && std::contiguous_iterator<typename Source::iterator_type>;

template <class Source>
concept modifiable_source = source<Source> && std::permutable<typename Source::unchecked_iterator_type>;

__RAZE_ALGORITHM_NAMESPACE_END