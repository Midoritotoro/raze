#pragma once 

#include <raze/RazeNamespace.h>
#include <raze/compatibility/CxxVersionDetection.h>

#include <type_traits>
#include <xutility>
#include <ranges>
#include <concepts>

__RAZE_TRAITS_NAMESPACE_BEGIN

template <class Iter>
constexpr inline bool is_iterator_volatile_v = std::is_volatile_v<std::remove_reference_t<std::iter_reference_t<Iter>>>;

template <class T, class = void>
constexpr inline bool is_iterator_v = false;

template <class T>
constexpr inline bool is_iterator_v<T, std::void_t<
	typename std::iterator_traits<T>::iterator_category>> = true;

template <class Iter, class = void>
constexpr bool allow_inheriting_unwrap_v = true;

template <class Iter>
constexpr bool allow_inheriting_unwrap_v<Iter, std::void_t<typename Iter::_Prevent_inheriting_unwrap>> =
    std::is_same_v<Iter, typename Iter::_Prevent_inheriting_unwrap>;

template <class Iter, class = void>
constexpr bool is_iterator_unwrappable_v = false;

template <class Iter>
constexpr bool is_iterator_unwrappable_v<Iter, std::void_t<decltype(std::declval<std::remove_cvref_t<Iter>&>()._Seek_to(
	std::declval<Iter>()._Unwrapped()))>> = allow_inheriting_unwrap_v<std::remove_cvref_t<Iter>>;

template <class Iter> 
constexpr bool is_iterator_unwrapped_v = !is_iterator_unwrappable_v<Iter>;

template <class Iter, class = void>
constexpr bool is_nothrow_unwrappable_v = false;

template <class Iter>
constexpr bool is_nothrow_unwrappable_v<Iter, std::void_t<decltype(std::declval<Iter>()._Unwrapped())>> =
    noexcept(std::declval<Iter>()._Unwrapped());

template <class Iter, class = bool>
constexpr bool can_unwrap_when_unverified_v = false;

template <class Iter>
constexpr bool can_unwrap_when_unverified_v<Iter, decltype(static_cast<bool>(Iter::_Unwrap_when_unverified))> =
    static_cast<bool>(Iter::_Unwrap_when_unverified);

template <class Iter>
constexpr bool is_possibly_unverified_iterator_unwrappable_v =
    is_iterator_unwrappable_v<Iter> && can_unwrap_when_unverified_v<std::remove_cvref_t<Iter>>;

template <class Iter, class = void>
constexpr bool is_offset_verifiable_v = false;

template <class Iter>
constexpr bool is_offset_verifiable_v<Iter, std::void_t<decltype(std::declval<const Iter&>()._Verify_offset(
	std::iter_difference_t<Iter>{}))>> = true;

template <class Iter, class = void>
constexpr bool is_offset_nothrow_verifiable_v = false;

template <class Iter>
constexpr bool is_offset_nothrow_verifiable_v <Iter, std::void_t<decltype(std::declval<const Iter&>()._Verify_offset(
	std::iter_difference_t<Iter>{}))>> = noexcept(std::declval<const Iter&>()._Verify_offset(std::iter_difference_t<Iter>{}));

template <class Iter>
constexpr bool is_iterator_unwrappable_for_offset_v = is_iterator_unwrappable_v<Iter> && 
	is_offset_verifiable_v<std::remove_cvref_t<Iter>>;

template <class Iter>
constexpr bool is_iterator_nothrow_unwrappable_for_offset_v = is_nothrow_unwrappable_v<Iter> &&
	is_offset_nothrow_verifiable_v<std::remove_cvref_t<Iter>>;

template <class Iter, class UIter, class = void>
constexpr bool is_wrapped_iterator_seekable_v = false;

template <class Iter, class UIter>
constexpr bool is_wrapped_iterator_seekable_v<Iter, UIter, 
	std::void_t<decltype(std::declval<Iter&>()._Seek_to(std::declval<UIter>()))>> = true;

template <class Iter, class UIter, class = void>
constexpr bool is_wrapped_iterator_nothrow_seekable_v = false;

template <class Iter, class UIter>
constexpr bool is_wrapped_iterator_nothrow_seekable_v<Iter, UIter, std::void_t<decltype(
	std::declval<Iter&>()._Seek_to(std::declval<UIter>()))>> = 
        noexcept(std::declval<Iter&>()._Seek_to(std::declval<UIter>()));

template <class Wrapped>
concept weakly_unwrappable = allow_inheriting_unwrap_v<std::remove_cvref_t<Wrapped>>
	&& requires(Wrapped&& wrapped) { std::forward<Wrapped>(wrapped)._Unwrapped(); };

template <class Sent>
concept weakly_unwrappable_sentinel = weakly_unwrappable<const std::remove_reference_t<Sent>&>;

template <class Iter>
concept weakly_unwrappable_iterator = weakly_unwrappable<Iter> &&
	requires(Iter&& it, std::remove_cvref_t<Iter>& mutable_it) {
		mutable_it._Seek_to(std::forward<Iter>(it)._Unwrapped());
	};

template <class Sent, class Iter>
concept unwrappable_sentinel_for = weakly_unwrappable_sentinel<Sent> && weakly_unwrappable_iterator<Iter> && 
	requires(Iter&& it, const std::remove_reference_t<Sent>& sent) {
		{ sent._Unwrapped() } -> std::sentinel_for<decltype(std::forward<Iter>(it)._Unwrapped())>;
	};

template <class Sent, class Iter>
raze_nodiscard raze_always_inline constexpr decltype(auto) uiter(Iter&& it)
	noexcept(!unwrappable_sentinel_for<Sent, Iter> || is_nothrow_unwrappable_v<Iter>)
{
	static_assert(std::sentinel_for<std::remove_cvref_t<Sent>, std::remove_cvref_t<Iter>>);

	if constexpr (std::is_pointer_v<std::remove_cvref_t<Iter>>) return it + 0;
	else if constexpr (unwrappable_sentinel_for<Sent, Iter>) return static_cast<Iter&&>(it)._Unwrapped();
	else return static_cast<Iter&&>(it);
}

template <class Iter, class Sent>
raze_nodiscard raze_always_inline constexpr decltype(auto) usent(Sent&& sent)
	noexcept(!unwrappable_sentinel_for<Sent, Iter> || is_nothrow_unwrappable_v<Sent>) 
{
	static_assert(std::sentinel_for<std::remove_cvref_t<Sent>, std::remove_cvref_t<Iter>>);

	if constexpr (std::is_pointer_v<std::remove_cvref_t<Sent>>) return sent + 0;
	else if constexpr (unwrappable_sentinel_for<Sent, Iter>) return static_cast<Sent&&>(sent)._Unwrapped();
	else return static_cast<Sent&&>(sent);
}

template <std::ranges::range R, class Iter>
raze_nodiscard raze_always_inline constexpr decltype(auto) r_uiter(Iter&& it)
	noexcept(noexcept(uiter<std::ranges::sentinel_t<R>>(static_cast<Iter&&>(it))))
{
	static_assert(std::same_as<std::remove_cvref_t<Iter>, std::ranges::iterator_t<R>>);
	return uiter<std::ranges::sentinel_t<R>>(static_cast<Iter&&>(it));
}

template <std::ranges::range R, class Sent>
raze_nodiscard raze_always_inline constexpr decltype(auto) r_usent(Sent&& sent)
	noexcept(noexcept(usent<std::ranges::iterator_t<R>>(static_cast<Sent&&>(sent))))
{
	static_assert(std::same_as<std::remove_cvref_t<Sent>, std::ranges::sentinel_t<R>>);
	return usent<std::ranges::iterator_t<R>>(static_cast<Sent&&>(sent));
}

template <class Sent, class Iter>
using ranges_unwrap_iter_t = std::remove_cvref_t<decltype(uiter<Sent>(std::declval<Iter>()))>;

template <class Sent, class Iter>
using ranges_unwrap_sent_t = std::remove_cvref_t<decltype(usent<Iter>(std::declval<Sent>()))>;

template <std::ranges::range Range>
using unwrapped_iterator_t = ranges_unwrap_iter_t<std::ranges::iterator_t<Range>, std::ranges::sentinel_t<Range>>;

template <std::ranges::range Range>
using unwrapped_sentinel_t = ranges_unwrap_sent_t<std::ranges::sentinel_t<Range>, std::ranges::iterator_t<Range>>;

#if defined(raze_cpp_msvc)
  constexpr inline auto ubegin = std::ranges::_Ubegin;
  constexpr inline auto uend = std::ranges::_Uend;
#else 
  constexpr inline auto ubegin = std::identity{};
  constexpr inline auto uend = std::identity{};
#endif // defined(raze_cpp_msvc)

template <std::forward_iterator Iter, class Sent>
raze_nodiscard raze_always_inline constexpr ranges_unwrap_iter_t<Iter, Sent> last_uiter(
	const ranges_unwrap_iter_t<Iter, Sent>& first, Sent&& last) 
		requires(std::sentinel_for<std::remove_cvref_t<Sent>, Iter>)
{
	  if constexpr (std::is_same_v<ranges_unwrap_iter_t<Iter, Sent>, ranges_unwrap_sent_t<Sent, Iter>>)
		  return usent<Iter>(std::forward<Sent>(last));
	  else return std::ranges::next(first, usent<Iter>(std::forward<Sent>(last)));
  }

template <std::ranges::forward_range R>
raze_nodiscard raze_always_inline constexpr auto last_uiter(R& r) {
	  if constexpr (std::ranges::common_range<R>) {
		  if constexpr (std::same_as<decltype(uend(r)), unwrapped_iterator_t<R>>) return uend(r);
		  else return usent<R>(std::ranges::end(r));
	  }
	  else if constexpr (std::ranges::sized_range<R>) return std::ranges::next(ubegin(r), std::ranges::distance(r));
	  else return std::ranges::next(ubegin(r), uend(r));
  }

template <class Result, class Wrapped, class Unwrapped>
raze_always_inline constexpr Result rewrap_subrange(Wrapped& v, std::ranges::subrange<Unwrapped>&& unwrapped_result) {
	if constexpr (std::same_as<Result, std::ranges::dangling>) {
		return std::ranges::dangling {};
	}
	else if constexpr (std::same_as<Result, std::ranges::subrange<Unwrapped>>) {
		return std::move(unwrapped_result);
	}
	else if constexpr (std::ranges::range<Wrapped>) {
		auto first = std::ranges::begin(v);
		auto last = first;

		first._Seek_to(unwrapped_result.begin());
		last._Seek_to(unwrapped_result.end());

		return Result { std::move(first), std::move(last) };
	}
	else {
		auto last = v;

		v._Seek_to(unwrapped_result.begin());
		last._Seek_to(unwrapped_result.end());

		return Result { std::move(v), std::move(last) };
	}
}

__RAZE_TRAITS_NAMESPACE_END
