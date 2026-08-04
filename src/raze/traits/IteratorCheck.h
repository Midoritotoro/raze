#pragma once 

#include <raze/RazeNamespace.h>
#include <raze/compatibility/CxxVersionDetection.h>

#include <type_traits>
#include <xutility>
#include <ranges>
#include <concepts>

__RAZE_TRAITS_NAMESPACE_BEGIN

template <class _Iterator_>
constexpr inline bool is_iterator_volatile_v = std::is_volatile_v<std::remove_reference_t<std::iter_reference_t<_Iterator_>>>;

template <class _Type_, class = void>
constexpr inline bool is_iterator_v = false;

template <class _Type_>
constexpr inline bool is_iterator_v<_Type_, std::void_t<
	typename std::iterator_traits<_Type_>::iterator_category>> = true;

template <class _Iterator_, class = void>
constexpr bool __allow_inheriting_unwrap_v = true;

template <class _Iterator_>
constexpr bool __allow_inheriting_unwrap_v<_Iterator_, std::void_t<typename _Iterator_::_Prevent_inheriting_unwrap>> =
    std::is_same_v<_Iterator_, typename _Iterator_::_Prevent_inheriting_unwrap>;

template <class _Iterator_, class _Sentinel_ = _Iterator_, class = void>
constexpr bool __is_range_verifiable_v = false;

template <class _Iterator_, class _Sentinel_>
constexpr bool __is_range_verifiable_v<_Iterator_, _Sentinel_, std::void_t<decltype(
	__verify_range__(std::declval<const _Iterator_&>(), std::declval<const _Sentinel_&>()))>> 
        = __allow_inheriting_unwrap_v<_Iterator_>;

template <class _Iterator_, class = void>
constexpr bool __is_iterator_unwrappable_v = false;

template <class _Iterator_>
constexpr bool __is_iterator_unwrappable_v<_Iterator_, std::void_t<decltype(std::declval<std::remove_cvref_t<_Iterator_>&>()._Seek_to(
	std::declval<_Iterator_>()._Unwrapped()))>> = __allow_inheriting_unwrap_v<std::remove_cvref_t<_Iterator_>>;

template <class _Iterator_> 
constexpr bool __is_iterator_unwrapped_v = !__is_iterator_unwrappable_v<_Iterator_>;

template <class _Iterator_, class = void>
constexpr bool __is_nothrow_unwrappable_v = false;

template <class _Iterator_>
constexpr bool __is_nothrow_unwrappable_v<_Iterator_, std::void_t<decltype(std::declval<_Iterator_>()._Unwrapped())>> =
    noexcept(std::declval<_Iterator_>()._Unwrapped());

template <class _Iterator_, class = bool>
constexpr bool __can_unwrap_when_unverified_v = false;

template <class _Iterator_>
constexpr bool __can_unwrap_when_unverified_v<_Iterator_, decltype(static_cast<bool>(_Iterator_::_Unwrap_when_unverified))> =
    static_cast<bool>(_Iterator_::_Unwrap_when_unverified);

template <class _Iterator_>
constexpr bool __is_possibly_unverified_iterator_unwrappable_v =
    __is_iterator_unwrappable_v<_Iterator_> && __can_unwrap_when_unverified_v<std::remove_cvref_t<_Iterator_>>;

template <class _Iterator_, class = void>
constexpr bool __is_offset_verifiable_v = false;

template <class _Iterator_>
constexpr bool __is_offset_verifiable_v <_Iterator_, std::void_t<decltype(std::declval<const _Iterator_&>()._Verify_offset(
	std::iter_difference_t<_Iterator_>{}))>> = true;

template <class _Iterator_, class = void>
constexpr bool __is_offset_nothrow_verifiable_v = false;

template <class _Iterator_>
constexpr bool __is_offset_nothrow_verifiable_v <_Iterator_, std::void_t<decltype(std::declval<const _Iterator_&>()._Verify_offset(
	std::iter_difference_t<_Iterator_>{}))>> = noexcept(std::declval<const _Iterator_&>()._Verify_offset(std::iter_difference_t<_Iterator_>{}));

template <class _Iterator_>
constexpr bool __is_iterator_unwrappable_for_offset_v = __is_iterator_unwrappable_v<_Iterator_> && 
	__is_offset_verifiable_v<std::remove_cvref_t<_Iterator_>>;

template <class _Iterator_>
constexpr bool __is_iterator_nothrow_unwrappable_for_offset_v =  __is_nothrow_unwrappable_v<_Iterator_> &&
	__is_offset_nothrow_verifiable_v<std::remove_cvref_t<_Iterator_>>;

template <class _Iterator_, class _UnwrappedIterator_, class = void>
constexpr bool __is_wrapped_iterator_seekable_v = false;

template <class _Iterator_, class _UnwrappedIterator_>
constexpr bool __is_wrapped_iterator_seekable_v<_Iterator_, _UnwrappedIterator_, 
	std::void_t<decltype(std::declval<_Iterator_&>()._Seek_to(std::declval<_UnwrappedIterator_>()))>> = true;

template <class _Iterator_, class _UnwrappedIterator_, class = void>
constexpr bool __is_wrapped_iterator_nothrow_seekable_v = false;

template <class _Iterator_, class _UnwrappedIterator_>
constexpr bool __is_wrapped_iterator_nothrow_seekable_v<_Iterator_, _UnwrappedIterator_, std::void_t<decltype(
	std::declval<_Iterator_&>()._Seek_to(std::declval<_UnwrappedIterator_>()))>> = 
        noexcept(std::declval<_Iterator_&>()._Seek_to(std::declval<_UnwrappedIterator_>()));

#if !defined(__verify_unchecked)
#  define __verify_unchecked(_Iterator) static_assert(raze::traits::__is_iterator_unwrapped_v<_Iterator>, "Iterators in unchecked-functions must be unwrapped. ");
#endif // !defined(__verify_unchecked)

template <class _Wrapped>
concept __weakly_unwrappable = __allow_inheriting_unwrap_v<std::remove_cvref_t<_Wrapped>>
	&& requires(_Wrapped&& __wrapped) { std::forward<_Wrapped>(__wrapped)._Unwrapped(); };

template <class _Sentinel_>
concept __weakly_unwrappable_sentinel = __weakly_unwrappable<const std::remove_reference_t<_Sentinel_>&>;

template <class _Iterator_>
concept __weakly_unwrappable_iterator = __weakly_unwrappable<_Iterator_> && \
	requires(_Iterator_&& __it, std::remove_cvref_t<_Iterator_>& __mutable_it) {
		__mutable_it._Seek_to(std::forward<_Iterator_>(__it)._Unwrapped());
	};

template <class _Sentinel_, class _Iterator_>
concept __unwrappable_sentinel_for = __weakly_unwrappable_sentinel<_Sentinel_> && __weakly_unwrappable_iterator<_Iterator_> && 
	requires(_Iterator_&& __it, const std::remove_reference_t<_Sentinel_>& __sentinel) {
		{ __sentinel._Unwrapped() } -> std::sentinel_for<decltype(std::forward<_Iterator_>(__it)._Unwrapped())>;
	};

template <class _Sentinel_, class _Iterator_>
raze_nodiscard raze_always_inline constexpr decltype(auto) __uiter(_Iterator_&& __iterator)
	noexcept(!__unwrappable_sentinel_for<_Sentinel_, _Iterator_> || __is_nothrow_unwrappable_v<_Iterator_>)
{
	static_assert(std::sentinel_for<std::remove_cvref_t<_Sentinel_>, std::remove_cvref_t<_Iterator_>>);

	if constexpr (std::is_pointer_v<std::remove_cvref_t<_Iterator_>>) return __iterator + 0;
	else if constexpr (__unwrappable_sentinel_for<_Sentinel_, _Iterator_>) return static_cast<_Iterator_&&>(__iterator)._Unwrapped();
	else return static_cast<_Iterator_&&>(__iterator);
}

template <class _Iterator_, class _Sentinel_>
raze_nodiscard raze_always_inline constexpr decltype(auto) __usent(_Sentinel_&& __sentinel)
	noexcept(!__unwrappable_sentinel_for<_Sentinel_, _Iterator_> || __is_nothrow_unwrappable_v<_Sentinel_>) 
{
	static_assert(std::sentinel_for<std::remove_cvref_t<_Sentinel_>, std::remove_cvref_t<_Iterator_>>);

	if constexpr (std::is_pointer_v<std::remove_cvref_t<_Sentinel_>>) return __sentinel + 0;
	else if constexpr (__unwrappable_sentinel_for<_Sentinel_, _Iterator_>) return static_cast<_Sentinel_&&>(__sentinel)._Unwrapped();
	else return static_cast<_Sentinel_&&>(__sentinel);
}

template <std::ranges::range _Range_, class _Iterator_>
raze_nodiscard raze_always_inline constexpr decltype(auto) __r_uiter(_Iterator_&& __iterator)
	noexcept(noexcept(__uiter<std::ranges::sentinel_t<_Range_>>(static_cast<_Iterator_&&>(__iterator))))
{
	static_assert(std::same_as<std::remove_cvref_t<_Iterator_>, std::ranges::iterator_t<_Range_>>);
	return __uiter<std::ranges::sentinel_t<_Range_>>(static_cast<_Iterator_&&>(__iterator));
}

template <std::ranges::range _Range_, class _Sentinel_>
raze_nodiscard raze_always_inline constexpr decltype(auto) __r_usent(_Sentinel_&& __sentinel)
	noexcept(noexcept(__usent<std::ranges::iterator_t<_Range_>>(static_cast<_Sentinel_&&>(__sentinel))))
{
	static_assert(std::same_as<std::remove_cvref_t<_Sentinel_>, std::ranges::sentinel_t<_Range_>>);
	return __usent<std::ranges::iterator_t<_Range_>>(static_cast<_Sentinel_&&>(__sentinel));
}

template <class _Iterator_, class _Sentinel_>
using __ranges_unwrap_iter_t = std::remove_cvref_t<decltype(__uiter<_Sentinel_>(std::declval<_Iterator_>()))>;

template <class _Sentinel_, class _Iterator_>
using __ranges_unwrap_sent_t = std::remove_cvref_t<decltype(__usent<_Iterator_>(std::declval<_Sentinel_>()))>;

template <std::ranges::range _Range_>
using __unwrapped_iterator_t = __ranges_unwrap_iter_t<std::ranges::iterator_t<_Range_>, std::ranges::sentinel_t<_Range_>>;

template <std::ranges::range _Range_>
using __unwrapped_sentinel_t = __ranges_unwrap_sent_t<std::ranges::sentinel_t<_Range_>, std::ranges::iterator_t<_Range_>>;

#if defined(raze_cpp_msvc)
  constexpr inline auto __ubegin = std::ranges::_Ubegin;
  constexpr inline auto __uend = std::ranges::_Uend;
#else 
  constexpr inline auto __ubegin = std::identity{};
  constexpr inline auto __uend = std::identity{};
#endif // defined(raze_cpp_msvc)

template <std::forward_iterator _Iterator_, class _Sentinel_>
raze_nodiscard raze_always_inline constexpr __ranges_unwrap_iter_t<_Iterator_, _Sentinel_> __last_uiter(
	const __ranges_unwrap_iter_t<_Iterator_, _Sentinel_>& __first, _Sentinel_&& __last) 
		requires(std::sentinel_for<std::remove_cvref_t<_Sentinel_>, _Iterator_>)
{
	  if constexpr (std::is_same_v<__ranges_unwrap_iter_t<_Iterator_, _Sentinel_>, __ranges_unwrap_sent_t<_Sentinel_, _Iterator_>>)
		  return __usent<_Iterator_>(std::forward<_Sentinel_>(__last));
	  else return std::ranges::next(__first, __usent<_Iterator_>(std::forward<_Sentinel_>(__last)));
  }

template <std::ranges::forward_range _Range_>
raze_nodiscard raze_always_inline constexpr auto __last_uiter(_Range_& __range) {
	  if constexpr (std::ranges::common_range<_Range_>) {
		  if constexpr (std::same_as<decltype(__uend(__range)), __unwrapped_iterator_t<_Range_>>) return __uend(__range);
		  else return __usent<_Range_>(std::ranges::end(__range));
	  }
	  else if constexpr (std::ranges::sized_range<_Range_>) return std::ranges::next(__ubegin(__range), std::ranges::distance(__range));
	  else return std::ranges::next(__ubegin(__range), __uend(__range));
  }

template <class _Result_, class _Wrapped_, class _Unwrapped_>
raze_always_inline constexpr _Result_ __rewrap_subrange(_Wrapped_& __v, std::ranges::subrange<_Unwrapped_>&& __unwrapped_result) {
	if constexpr (std::same_as<_Result_, std::ranges::dangling>) {
		return std::ranges::dangling {};
	}
	else if constexpr (std::same_as<_Result_, std::ranges::subrange<_Unwrapped_>>) {
		return std::move(__unwrapped_result);
	}
	else if constexpr (std::ranges::range<_Wrapped_>) {
		auto __first = std::ranges::begin(__v);
		auto __last = __first;

		__first._Seek_to(__unwrapped_result.begin());
		__last._Seek_to(__unwrapped_result.end());

		return _Result_ { std::move(__first), std::move(__last) };
	}
	else {
		auto __last = __v;

		__v._Seek_to(__unwrapped_result.begin());
		__last._Seek_to(__unwrapped_result.end());

		return _Result_ { std::move(__v), std::move(__last) };
	}
}

__RAZE_TRAITS_NAMESPACE_END
