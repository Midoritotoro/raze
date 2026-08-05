#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Range_>
struct range_data_source {
	using iterator_type = decltype(std::ranges::begin(std::declval<_Range_>()));
	using sentinel_type = decltype(std::ranges::end(std::declval<_Range_>()));

	using unchecked_iterator_type = decltype(traits::__ubegin(std::declval<_Range_>()));
	using unchecked_sentinel_type = decltype(traits::__uend(std::declval<_Range_>()));

	constexpr explicit range_data_source(const _Range_& __r) noexcept :
		_it(std::ranges::begin(__r)), _sent(std::ranges::end(__r))
	{}

	raze_nodiscard raze_always_inline constexpr bool empty() const noexcept {
		return _it == _sent;
	}

	raze_nodiscard raze_always_inline constexpr auto size() const noexcept {
		if constexpr (constexpr_sized_range<_Range_>) return __bytes_distance(options::as<_Range_>{});
		else return __bytes_distance(_it, _sent);
	}

	raze_nodiscard raze_always_inline constexpr iterator_type begin() const noexcept {
		return _it;
	}

	raze_nodiscard raze_always_inline constexpr sentinel_type end() const noexcept {
		return _sent;
	}

	raze_nodiscard raze_always_inline constexpr unchecked_iterator_type ubegin() const noexcept {
		return traits::__r_uiter<_Range_>(_it);
	}

	raze_nodiscard raze_always_inline constexpr unchecked_sentinel_type uend() const noexcept {
		return traits::__r_usent<_Range_>(_sent);
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(iterator_type __it) noexcept
	{
		return traits::__r_uiter<_Range_>(std::move(__it));
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(std::iter_value_t<unchecked_iterator_type>* __ptr) noexcept 
	{
		unchecked_iterator_type __it;
		__seek_iter(__it, __ptr);
		return __it;
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(const std::iter_value_t<unchecked_iterator_type>* __ptr) noexcept 
	{
		unchecked_iterator_type __it;
		__seek_iter(__it, __ptr);
		return __it;
	}

	raze_nodiscard raze_always_inline constexpr iterator_type
		wrap(unchecked_iterator_type __uit) const noexcept
	{
		iterator_type __it = _it;
		__seek_iter(__it, __uit);
		return __it;
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& __uit,
		std::iter_value_t<unchecked_iterator_type>* __ptr) noexcept
	{
		__seek_iter(__uit, __ptr);
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& __uit,
		const std::iter_value_t<unchecked_iterator_type>* __ptr) noexcept
	{
		__seek_iter(__uit, __ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& __uit,
		std::iter_value_t<iterator_type>* __ptr) noexcept
	{
		__seek_iter(__uit, __ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& __uit,
		const std::iter_value_t<iterator_type>* __ptr) noexcept
	{
		__seek_iter(__uit, __ptr);
	}

	static consteval bool constexpr_sized() noexcept {
		return constexpr_sized_range<_Range_>;
	}

	iterator_type _it;
	sentinel_type _sent;
};

template <class _Iterator_, class _Sentinel_>
struct iter_data_source {
	using iterator_type = _Iterator_;
	using sentinel_type = _Sentinel_;

	using unchecked_iterator_type = decltype(traits::__uiter<_Sentinel_>(std::declval<_Iterator_>()));
	using unchecked_sentinel_type = decltype(traits::__usent<_Iterator_>(std::declval<_Sentinel_>()));

	constexpr iter_data_source(_Iterator_ __it, _Sentinel_ __sent) noexcept :
		_it(std::move(__it)), _sent(std::move(__sent))
	{}

	raze_nodiscard raze_always_inline constexpr bool empty() const noexcept {
		return _it == _sent;
	}

	raze_nodiscard raze_always_inline constexpr auto size() const noexcept {
		return __bytes_distance(ubegin(), uend());
	}

	raze_nodiscard raze_always_inline constexpr iterator_type begin() const noexcept {
		return _it;
	}

	raze_nodiscard raze_always_inline constexpr sentinel_type end() const noexcept {
		return _sent;
	}

	raze_nodiscard raze_always_inline constexpr unchecked_iterator_type ubegin() const noexcept {
		return traits::__uiter<_Sentinel_>(_it);
	}

	raze_nodiscard raze_always_inline constexpr unchecked_sentinel_type uend() const noexcept {
		return traits::__usent<_Iterator_>(_sent);
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(iterator_type __it) noexcept 
	{
		return traits::__uiter<_Sentinel_>(std::move(__it));
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(std::iter_value_t<unchecked_iterator_type>* __ptr) noexcept 
	{
		unchecked_iterator_type __it;
		__seek_iter(__it, __ptr);
		return __it;
	}

	raze_nodiscard static raze_always_inline constexpr unchecked_iterator_type
		unwrap(const std::iter_value_t<unchecked_iterator_type>* __ptr) noexcept
	{
		unchecked_iterator_type __it;
		__seek_iter(__it, __ptr);
		return __it;
	}

	raze_nodiscard raze_always_inline constexpr iterator_type
		wrap(unchecked_iterator_type __uit) const noexcept
	{
		iterator_type __it = _it;
		__seek_iter(__it, __uit);
		return __it;
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& __uit,
		std::iter_value_t<unchecked_iterator_type>* __ptr) noexcept 
	{
		__seek_iter(__uit, __ptr);
	}

	static raze_always_inline constexpr void from_ptr(unchecked_iterator_type& __uit,
		const std::iter_value_t<unchecked_iterator_type>* __ptr) noexcept
	{
		__seek_iter(__uit, __ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& __uit,
		std::iter_value_t<iterator_type>* __ptr) noexcept
	{
		__seek_iter(__uit, __ptr);
	}

	static raze_always_inline constexpr void from_ptr(iterator_type& __uit,
		const std::iter_value_t<iterator_type>* __ptr) noexcept
	{
		__seek_iter(__uit, __ptr);
	}

	static consteval bool constexpr_sized() noexcept {
		return false;
	}

	iterator_type _it;
	sentinel_type _sent;
};

template <class _Range_>
raze_nodiscard raze_always_inline constexpr auto get_source(_Range_&& __r) noexcept {
	return range_data_source<_Range_>(std::forward<_Range_>(__r));
}

template <class _Iterator_, class _Sentinel_>
raze_nodiscard raze_always_inline constexpr auto get_source(_Iterator_ __it, _Sentinel_ __sent) noexcept {
	return iter_data_source<_Iterator_, _Sentinel_>(std::move(__it), std::move(__sent));
}

template <class _Source_>
concept source = requires(
	_Source_ __src,
	typename _Source_::iterator_type __it,
	typename _Source_::unchecked_iterator_type __uit,
	std::iter_value_t<typename _Source_::unchecked_iterator_type>*__ptr,
	const std::iter_value_t<typename _Source_::unchecked_iterator_type>*__cptr)
{
	typename _Source_::iterator_type;
	typename _Source_::sentinel_type;

	typename _Source_::unchecked_iterator_type;
	typename _Source_::unchecked_sentinel_type;

	{ __src.empty() } noexcept -> std::convertible_to<bool>;
	{ __src.size() } noexcept -> std::convertible_to<sizetype>;

	{ __src.begin() } noexcept -> std::same_as<typename _Source_::iterator_type>;
	{ __src.end() } noexcept -> std::same_as<typename _Source_::sentinel_type>;
	{ __src.ubegin() } noexcept -> std::same_as<typename _Source_::unchecked_iterator_type>;
	{ __src.uend() } noexcept -> std::same_as<typename _Source_::unchecked_sentinel_type>;

	{ _Source_::unwrap(__it) } noexcept -> std::same_as<typename _Source_::unchecked_iterator_type>;
	{ _Source_::unwrap(__ptr) } noexcept -> std::same_as<typename _Source_::unchecked_iterator_type>;

	{ _Source_::unwrap(__cptr) } noexcept -> std::same_as<typename _Source_::unchecked_iterator_type>;
	{ __src.wrap(__uit) } noexcept -> std::same_as<typename _Source_::iterator_type>;

	{ _Source_::from_ptr(__uit, __ptr) } noexcept -> std::same_as<void>;
	{ _Source_::from_ptr(__uit, __cptr) } noexcept -> std::same_as<void>;

	{ _Source_::from_ptr(__it, __ptr) } noexcept -> std::same_as<void>;
	{ _Source_::from_ptr(__it, __cptr) } noexcept -> std::same_as<void>;
};

__RAZE_ALGORITHM_NAMESPACE_END